"""
data schema:
{
    "data_raw" : int,    # raw data
    "data_filtered: int, # low pass filter
    "bpm" : int,         # beats per minute
    "created_at" :       # timestamp
}
"""
import numpy as np
from pymongo import MongoClient
from bson.json_util import dumps, loads
from bson.objectid import ObjectId
import datetime

class services:
    def __init__(self, db, cluster_name, mongo_uri = 'mongodb://localhost:27017/', filter_sample_size = 10, cache_size = 100):
        # database
        self.db = db
        self.collection = db[cluster_name]
        
        # cache
        self.cache = np.array([], dtype=object) # this should've been a linked list, but I'm too lazy
        
        # low pass filter
        self.filter_sample_size = filter_sample_size
        self.cache_max_size = cache_size
        self.tail_index = 0
        
    # utility functions
    def filtered(self, data_raw):
        """
        Apply a low-pass filter to the raw data.
        :param data_raw: Array of raw data.
        :return: Filtered data.
        """
        if self.cache.size < self.filter_sample_size:
            return np.mean(np.append(np.array([data['data_raw'] for data in self.cache]), data_raw))
        elif self.cache.size == self.filter_sample_size:
            return np.mean(np.append(np.array([data['data_raw'] for data in self.cache[-(self.filter_sample_size-1):]]), data_raw))
        else:
            average = self.cache[-1]['data_filtered'] + (data_raw - self.cache[self.tail_index]['data_raw']) / self.filter_sample_size

            self.tail_index += 1
            if self.tail_index == self.cache.size:
                self.tail_index -= self.filter_sample_size
            return average
    
    def get_bpm(self, data_raw, data_filtered, created_at):
        """
        Calculate BPM from recent data in the cache.
        Assumes data is sampled at roughly regular intervals (e.g., 100Hz or 50Hz).
        """
        if len(self.cache) < 3:
            return 0  # Not enough data to calculate BPM
        
        # Extract filtered values and timestamps
        filtered_values = np.append(np.array([data['data_filtered'] for data in self.cache]), data_filtered)
        timestamps = np.append(np.array([data['created_at'].timestamp() for data in self.cache]), created_at.timestamp())

        # Peak detection: a point is a peak if it's higher than its neighbors and above a threshold (I use mean of filtered values because im lazy)
        peaks = []
        threshold = np.mean(filtered_values)
        for i in range(1, len(filtered_values) - 1):
            if filtered_values[i - 1] < filtered_values[i] > filtered_values[i + 1] and filtered_values[i] > threshold:
                peaks.append(timestamps[i])

        # Need at least two peaks to measure time between them
        if len(peaks) < 2:
            return 0

        # Compute time differences between peaks
        intervals = np.diff(peaks)  # seconds
        if np.any(intervals == 0):
            return 0  # Avoid division by zero

        # Average interval between heartbeats
        avg_interval = np.mean(intervals)  # seconds per beat
        bpm = 60 / avg_interval

        return int(bpm)
        
    
    # API services
    def create(self, data_raw):
        """
        Create a new data entry in the collection.
        :param data: Dictionary containing the data to be inserted.
        :return: The ID of the inserted document.
        """
        # new data
        new_data = dict()
        new_data['data_raw'] = data_raw
        new_data['data_filtered'] = self.filtered(data_raw)
        new_data['created_at'] = datetime.datetime.now()
        new_data['bpm'] = self.get_bpm(data_raw, new_data['data_filtered'], new_data['created_at'])
        
        # handle raw caching
        self.cache = np.append(self.cache, new_data)
        if self.cache.size > self.cache_max_size:
            self.cache = np.delete(self.cache, 0)  # remove the oldest entry
        
        # insert to database
        result = self.collection.insert_one(new_data)
        return result
    
    def read(self, limit=-1):
        """
        Read data from the collection or cache.
        :param limit: Limit the number of documents returned. If -1, return all documents.
        :return: List of documents in JSON format.
        """
        if self.cache.size < limit and limit > 0:
            return dumps(self.collection.find().sort("created_at", -1).limit(limit))
        elif limit <= self.cache.size and limit > 0:
            return dumps(self.cache[-limit:])
        elif limit == -1:
            return dumps(self.collection.find().sort("created_at", -1))
        else:
            return dumps(self.collection.find().sort("created_at", -1).limit(limit))
    
    def clear(self):
        """
        Clear the collection and cache.
        :return: Number of documents deleted.
        """
        result = self.collection.delete_many({})
        self.cache = np.array([], dtype=object)
        self.tail_index = 0
        return result