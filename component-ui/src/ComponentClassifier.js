import React, { useState } from "react";

function ComponentClassifier() {
  const [file, setFile] = useState(null);
  const [preview, setPreview] = useState(null);
  const [result, setResult] = useState(null);
  const [loading, setLoading] = useState(false);

  const handleFileChange = (e) => {
    const selectedFile = e.target.files[0];
    if (selectedFile) {
      setFile(selectedFile);
      setPreview(URL.createObjectURL(selectedFile));
      setResult(null);
    }
  };

  const handleSubmit = async () => {
    if (!file) {
      alert("Please upload an image first.");
      return;
    }

    const formData = new FormData();
    formData.append("image", file);

    setLoading(true);
    try {
      const res = await fetch("http://127.0.0.1:5000/predict", {
        method: "POST",
        body: formData,
      });

      const data = await res.json();

      if (!res.ok) {
        setResult({ error: data.error || "Unknown error" });
      } else {
        setResult(data);
      }
    } catch (error) {
      setResult({ error: "Failed to connect to backend." });
    }
    setLoading(false);
  };

  return (
    <div className="flex min-h-screen bg-gray-50 font-sans">
      {/* Sidebar */}
      <aside className="w-64 bg-blue-700 text-white p-6 hidden md:flex flex-col">
        <h1 className="text-3xl font-bold mb-10">🔧 MechNet</h1>
        <nav className="space-y-4 text-lg">
          <div className="font-semibold text-blue-100">
            Component Classifier
          </div>
          <div className="hover:text-blue-200 cursor-pointer">
            Crack Detection
          </div>
          <div className="hover:text-blue-200 cursor-pointer">Settings</div>
        </nav>
        <div className="mt-auto text-xs text-blue-200 pt-10">
          © 2025 MechNet Inc.
        </div>
      </aside>

      {/* Main Content */}
      <div className="flex-1 flex flex-col">
        {/* Header */}
        <header className="bg-white shadow px-8 py-4">
          <h2 className="text-2xl font-semibold">Component Classifier</h2>
        </header>

        {/* Classifier Section */}
        <main className="flex-1 p-6 sm:p-10 bg-gray-100">
          <div className="bg-white p-8 rounded-xl shadow-md max-w-3xl mx-auto">
            <h3 className="text-xl font-semibold mb-4">
              Step 1: Upload an Image
            </h3>
            <input
              type="file"
              accept="image/*"
              onChange={handleFileChange}
              className="block w-full text-sm text-gray-700 mb-6 file:mr-4 file:py-2 file:px-4
              file:rounded-md file:border-0 file:text-sm file:font-semibold
              file:bg-blue-100 file:text-blue-700 hover:file:bg-blue-200"
            />

            {preview && (
              <>
                <h4 className="text-lg font-medium mb-2">Image Preview:</h4>
                <div className="mb-6 flex justify-center">
                  <img
                    src={preview}
                    alt="Preview"
                    className="max-w-[300px] max-h-[250px] rounded-md shadow border border-gray-300 object-contain"
                  />
                </div>
              </>
            )}

            <h4 className="text-lg font-medium mb-2">Step 2: Run Prediction</h4>
            <div className="mb-6">
              <button
                onClick={handleSubmit}
                disabled={loading || !file}
                className={`w-full py-3 rounded-lg text-white font-medium text-lg transition ${
                  loading || !file
                    ? "bg-gray-400 cursor-not-allowed"
                    : "bg-blue-600 hover:bg-blue-700"
                }`}
              >
                {loading ? "Processing..." : "Predict"}
              </button>
            </div>

            {result && (
              <div className="mt-4 p-4 rounded-md border border-gray-200 bg-gray-50 text-center">
                {result.error ? (
                  <p className="text-red-600 font-medium">
                    ⚠️ Error: {result.error}
                  </p>
                ) : (
                  <>
                    <p className="text-xl">
                      <strong>Prediction:</strong>{" "}
                      <span className="text-blue-700 font-semibold">
                        {result.label}
                      </span>
                    </p>
                    <p className="text-lg">
                      <strong>Confidence:</strong>{" "}
                      <span className="text-green-600 font-semibold">
                        {Math.round(result.confidence * 100)}%
                      </span>
                    </p>
                  </>
                )}
              </div>
            )}
          </div>
        </main>
      </div>
    </div>
  );
}

export default ComponentClassifier;
