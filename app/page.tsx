"use client";
import { Fragment, useEffect, useState } from "react";

/**
 * helper to fill 16x16 canvas with zeros, canvas is represented by 256 staring
 */
const generateZerosString = () => {
  let zerosString = "";

  for (let i = 0; i < 16; i++) {
    for (let j = 0; j < 16; j++) {
      zerosString += "0";
    }
  }

  return zerosString;
};

const apiUrl = "http://192.168.0.113";

export default function Home() {
  const [canvas, setCanvas] = useState(generateZerosString());
  const [drawingMode, setDrawingMode] = useState(false);
  const [name, setName] = useState("main");
  const [animations, setAnimations] = useState<Record<string, string[]>>({});

  const toggleCanvasPixel = (
    y: number,
    x: number,
    ignoreDrawingMode?: boolean,
  ) => {
    if (drawingMode || ignoreDrawingMode)
      setCanvas((prevState) => {
        let strArray = prevState.split("");

        const targetIndex = y * 16 + x;
        strArray[targetIndex] = prevState[targetIndex] === "0" ? "1" : "0";

        return strArray.join("");
      });
  };

  const send = async (str: string) => {
    await fetch(apiUrl + "/?text=" + str, {
      method: "POST",
    });
  };

  /**
   * add canvas to array of canvas / animation
   */
  const add = () => {
    setAnimations((prevState) => {
      let frames: string[] = [];
      if (name in prevState && prevState[name]) {
        frames = prevState[name];
      }

      // push new frame
      frames.push(canvas);

      return {
        ...prevState,
        [name]: frames,
      };
    });
  };

  /**
   * send array of frames (256 strings seperated by ;) to server
   * @param animationName
   */
  const sendAnimation = async (animationName: string) => {
    const str = ";" + Object.values(animations[animationName]).join(";");

    await fetch(apiUrl + "/set", {
      method: "POST",
      body: JSON.stringify({
        text: str,
      }),
    });
  };

  // load animation from server
  useEffect(() => {
    (async () => {
      let res = await fetch(apiUrl + "/read", {
        method: "GET",
      });

      let text = await res.text();
      let canvasArray = text.split(";");

      // remove first element
      canvasArray.shift();

      setAnimations({
        main: canvasArray,
      });
    })();
  }, []);

  return (
    <main
      className="flex items-center overflow-y-scroll flex-row gap-8"
      onMouseDown={() => setDrawingMode(true)}
      onMouseUp={() => setDrawingMode(false)}
    >
      <div>
        {drawingMode ? "Drawing" : "not drawing"}
        <div className="grid grid-cols-16 gap-2 z-40">
          {Array.from({ length: 16 }).map((_, rowIndex) => (
            <Fragment key={`row-${rowIndex}`}>
              {Array.from({ length: 16 }).map((_, colIndex) => (
                <div
                  key={`cell-${rowIndex}-${colIndex}`}
                  className={`cursor-pointer h-8 w-8 flex items-center justify-center ${
                    canvas.charAt(rowIndex * 16 + colIndex) == "0"
                      ? "bg-white"
                      : "bg-emerald-400"
                  }`}
                  onClick={() => toggleCanvasPixel(rowIndex, colIndex, true)}
                  onPointerEnter={(event) => {
                    if (event.pointerType == "pen")
                      toggleCanvasPixel(rowIndex, colIndex, true);

                    if (event.pointerType == "mouse")
                      toggleCanvasPixel(rowIndex, colIndex);
                  }}
                >
                  {/* Display cell content here */}
                </div>
              ))}
            </Fragment>
          ))}
        </div>
        <div onClick={() => send(canvas)}>Canvas Senden</div>
        <input
          onChange={(e) => setName(e.target.value)}
          value={name}
          type={"text"}
          placeholder="Name"
        />
        <div onClick={add}>Add to animation</div>
      </div>
      <div className="w-[50%]">
        <div className="flex flex-col bg-slate-600 p-2">
          {Object.entries(animations).map(([key, val]) => (
            <Fragment key={key}>
              <div>
                {key || "Kein"}{" "}
                <span
                  className="cursor-pointer underline"
                  onClick={() => sendAnimation(key)}
                >
                  (Animation Senden)
                </span>
              </div>
              <div className="flex flex-col mb-4 overflow-x-scroll">
                <div className="flex flex-row self-start">
                  {val.map((item, index) => (
                    <div key={index}>
                      <div>
                        ({index})
                        <span
                          onClick={() => setCanvas(item)}
                          className="ml-1 mr-1 cursor-pointer underline"
                        >
                          load to canvas
                        </span>
                        <span
                          className="cursor-pointer underline"
                          onClick={() => {
                            // override frame with canvas
                            setAnimations((prevState) => {
                              let prevAnimation = prevState[key];
                              prevAnimation[index] = canvas;
                              return {
                                ...prevState,
                                [key]: prevAnimation,
                              };
                            });
                          }}
                        >
                          load canvas to frame (overrides frame)
                        </span>
                      </div>
                      <div className="grid grid-cols-16 gap-1 mr-4 w-60 h-60">
                        {Array.from({ length: 16 }).map((_, rowIndex) => (
                          <Fragment key={`row-${rowIndex}`}>
                            {Array.from({ length: 16 }).map((_, colIndex) => (
                              <div
                                key={`cell-${rowIndex}-${colIndex}`}
                                className={`block items-center justify-center ${
                                  item.charAt(rowIndex * 16 + colIndex) == "0"
                                    ? "bg-white"
                                    : "bg-emerald-400"
                                }`}
                              />
                            ))}
                          </Fragment>
                        ))}
                      </div>
                    </div>
                  ))}
                </div>
              </div>
            </Fragment>
          ))}
        </div>
      </div>
    </main>
  );
}
