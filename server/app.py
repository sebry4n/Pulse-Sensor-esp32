from flask import Flask, request, jsonify
from pymongo import MongoClient
from bson.json_util import dumps, loads
from bson.objectid import ObjectId

app = Flask(__name__)
client = MongoClient('mongodb://localhost:27017/')
db = client['admin']  # Replace 'testdb' with your MongoDB database name
collection = db['numbers']  # Replace 'numbers' with your collection name

# CRUD operations

# Create
@app.route('/numbers', methods=['POST'])
def create_number():
    data = request.get_json()
    num = data['num']
    new_number = {"num": num}
    result = collection.insert_one(new_number)
    return jsonify({"message": "Number added successfully", "id": str(result.inserted_id)}), 201

# Read
@app.route('/numbers', methods=['GET'])
def get_all_numbers():
    numbers = collection.find()
    return dumps(numbers), 200

@app.route('/numbers/<id>', methods=['GET'])
def get_number(id):
    number = collection.find_one({"_id": ObjectId(id)})
    if number:
        return dumps(number), 200
    else:
        return jsonify({"error": "Number not found"}), 404

# Update
@app.route('/numbers/<id>', methods=['PUT'])
def update_number(id):
    data = request.get_json()
    updated_num = data['num']
    result = collection.update_one({"_id": ObjectId(id)}, {"$set": {"num": updated_num}})
    if result.modified_count > 0:
        return jsonify({"message": "Number updated successfully"}), 200
    else:
        return jsonify({"error": "Number not found"}), 404

# Delete
@app.route('/numbers/<id>', methods=['DELETE'])
def delete_number(id):
    result = collection.delete_one({"_id": ObjectId(id)})
    if result.deleted_count > 0:
        return jsonify({"message": "Number deleted successfully"}), 200
    else:
        return jsonify({"error": "Number not found"}), 404

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000, debug=True)
