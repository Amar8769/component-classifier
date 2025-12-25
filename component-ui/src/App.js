import React, { useState, useEffect } from "react";
import "./App.css";

function App() {
  const [selectedImage, setSelectedImage] = useState(null);
  const [imageFile, setImageFile] = useState(null);
  const [prediction, setPrediction] = useState(null);
  const [loading, setLoading] = useState(false);
  const [isDark, setIsDark] = useState(false);

  useEffect(() => {
    document.body.classList.toggle("dark", isDark);
  }, [isDark]);

  const handleImageUpload = (file) => {
    if (file) {
      setImageFile(file);
      setSelectedImage(URL.createObjectURL(file));
      setPrediction(null);
    }
  };

  const handleFileInputChange = (e) => {
    handleImageUpload(e.target.files[0]);
  };

  const handleDrop = (e) => {
    e.preventDefault();
    handleImageUpload(e.dataTransfer.files[0]);
  };

  const handleDragOver = (e) => {
    e.preventDefault();
  };

  const handlePredictClick = () => {
    if (!imageFile) return;

    const formData = new FormData();
    formData.append("image", imageFile);

    setLoading(true);
    fetch("http://localhost:5000/predict", {
      method: "POST",
      body: formData,
    })
      .then((res) => res.json())
      .then((data) => {
        setPrediction(data);
        setLoading(false);
      })
      .catch((err) => {
        console.error("Prediction error:", err);
        setLoading(false);
      });
  };

  const toggleDarkMode = () => setIsDark((prev) => !prev);

  return (
    <div className="app-container">
      <button className="dark-toggle" onClick={toggleDarkMode}>
        {isDark ? "☀ Light Mode" : "🌙 Dark Mode"}
      </button>

      <h1 className="title">MechNet</h1>

      <div className="section">
        <h2>Component Classifier</h2>

        <div
          className="drop-zone"
          onDrop={handleDrop}
          onDragOver={handleDragOver}
        >
          <p>Drag and drop an image here or</p>
          <input type="file" onChange={handleFileInputChange} />
        </div>

        {selectedImage && (
          <>
            <div className="image-preview">
              <img src={selectedImage} alt="Preview" />
            </div>
            <button onClick={handlePredictClick} disabled={loading}>
              {loading ? "Predicting..." : "Predict"}
            </button>
          </>
        )}

        {loading && (
          <div className="spinner">
            <div className="loader"></div>
            <p>Analyzing image...</p>
          </div>
        )}

        {prediction && !loading && (
          <div className="result-box">
            <p>
              <strong>Prediction:</strong> {prediction.label}
            </p>
            <p>
              <strong>Confidence:</strong>{" "}
              {(prediction.confidence * 100).toFixed(2)}%
            </p>
          </div>
        )}
      </div>
    </div>
  );
}

export default App;
