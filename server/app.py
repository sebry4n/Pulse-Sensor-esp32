"""
data schema:
{
    "data_raw" : int,    # raw data
    "data_filtered: int, # low pass filter
    "bpm" : int,         # beats per minute
    "created_at" :       # timestamp
}
"""
from flask import Flask, request, jsonify
from pymongo import MongoClient
from bson.json_util import dumps, loads
from bson.objectid import ObjectId
from services import services

DB_NAME = 'jarkom_pulse'
CLUSTER_NAME = 'data'
PORT = 5000
DEBUG = True
MONGO_URI = 'mongodb://localhost:27017/'

app = Flask(__name__)
client = MongoClient(MONGO_URI)
db = client[DB_NAME]
collection = db[CLUSTER_NAME]
service = services(db, CLUSTER_NAME, MONGO_URI)

# CRUD operations

# Create
@app.route('/data/publish_raw', methods=['POST'])
def create_data():
    data = request.get_json()
    data_raw = data['reading']
    result = service.create(data_raw)
    
    # for debugging purposes
    print(f"Received data: {data_raw}")
    print(f"Processed data: {result}")
    
    return jsonify({"message": "Data added successfully", "id": str(result.inserted_id)}), 201

# Read
@app.route('/data', methods=['GET'])
def get_data():
    limit = request.args.get('limit', default=-1, type=int)
    return service.read(limit), 200

@app.route('/data/<id>', methods=['GET'])
def get_data_by_id(id):
    data = collection.find_one({"_id": ObjectId(id)})
    if data:
        return dumps(data), 200
    else:
        return jsonify({"error": "Data not found"}), 404

# Delete
@app.route('/data/clear', methods=['DELETE'])
def delete_many():
    result = service.clear()
    return jsonify({"message": f"{result.deleted_count} records deleted"}), 200

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=PORT, debug=DEBUG)
