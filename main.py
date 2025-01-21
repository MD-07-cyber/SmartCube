from flask import Flask, request, jsonify
from deepface import DeepFace
import cv2
import numpy as np

app = Flask(__name__)

@app.route('/mood', methods=['POST'])
def mood_detection():
    try:
        # Receive image
        file_bytes = request.data
        np_arr = np.frombuffer(file_bytes, np.uint8)
        img = cv2.imdecode(np_arr, cv2.IMREAD_COLOR)

        # Perform emotion analysis
        analysis = DeepFace.analyze(img, actions=['emotion'], enforce_detection=False)
        emotion = analysis['dominant_emotion']

        # Generate response based on emotion
        messages = {
            "happy": "You look happy! Keep smiling 😊",
            "sad": "It's okay to feel sad. Things will get better! 💙",
            "angry": "Take a deep breath. Stay calm! 🧘",
            "fear": "Don't be afraid! You're stronger than you think 💪",
            "surprise": "Wow, you seem surprised! What's the good news? 🎉",
            "neutral": "You seem neutral. Hope you're having a steady day! 🙂",
            "disgust": "Is something bothering you? Let’s shake it off! 🌟"
        }
        message = messages.get(emotion, "I'm here for you, no matter what! ❤️")

        # Send message back
        return jsonify({"message": message})

    except Exception as e:
        print(f"Error: {e}")
        return jsonify({"message": "Error processing image"}), 500

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)
