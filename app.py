from flask import Flask, request, jsonify
import pickle

# Load the saved model
model = pickle.load(open('rain_prediction_model.pkl', 'rb'))

app = Flask(__name__)

@app.route('/predict', methods=['POST'])
def predict():
    data = request.json
    features = [[data['Temperature (C)'], data['Humidity'], data['Wind Speed (km/h)']]]
    prediction = model.predict(features)
    return jsonify({'rain_prediction': int(prediction[0])})

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)
