from flask import Flask, request, jsonify
from deepface import DeepFace
import cv2

app = Flask(__name__)

@app.route("/detect_mood", methods=["POST"])
def detect_mood():
    try:
        # Check if the image is in the request
        if 'image' not in request.files:
            return jsonify({"error": "No image provided"}), 400

        # Save the received image to a file
        file = request.files['image']
        file_path = "received_image.jpg"
        file.save(file_path)

        # Load the image using OpenCV
        img = cv2.imread(file_path)

        # Analyze the mood/emotion using DeepFace
        analysis = DeepFace.analyze(img_path=file_path, actions=['emotion'], enforce_detection=False)

        # Get the dominant emotion
        emotion = analysis['dominant_emotion']

        # Return the detected mood
        return jsonify({"mood": emotion})

    except Exception as e:
        return jsonify({"error": str(e)}), 500

if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000)
# The serial communication code has been moved to a separate file named serial_communication.py
