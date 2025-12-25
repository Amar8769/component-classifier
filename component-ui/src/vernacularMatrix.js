import { useState } from "react";
import axios from "axios";

function VernacularMatrix() {
  const [matrix, setMatrix] = useState("");

  const generateMatrix = async () => {
    const response = await axios.post("http://localhost:5000/vernacular", {
      text: "की",
      width: 20,
      height: 15,
      default_char: "#",
      custom: "1 - $,2 - @",
    });
    setMatrix(response.data.matrix);
  };

  return (
    <div>
      <button onClick={generateMatrix}>Generate</button>
      <pre className="font-mono whitespace-pre-wrap mt-4 p-4 bg-gray-100 rounded">
        {matrix}
      </pre>
    </div>
  );
}

export default VernacularMatrix;
