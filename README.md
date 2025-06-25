# Project Jarkom dashboard sensor heart pulse
Project akhir untuk mata kuliah Jaringan Komputer, dengan topik sensor detak jantung yang terhubung ke server menggunakan Flask dan MongoDB serta dapat diakses melalui dashboard berbasis web menggunakan NextJS.

## Instalasi
### 1. Setup Database
Database menggunakan MongoDB. Dianjurkan menggunakan [mongodb compass](https://www.mongodb.com/try/download/community) supaya lebih mudah. Tidak perlu buat cluster ataupun collection karena server backend akan secara automatis membuatnya. Nama cluster default adalah `jarkom_pulse`, dan collection default adalah `data`.
### 2. Setup Backend
Cukup install dependency
```bash
pip install flask pymongo flask-cors
```
Lalu jalankan server Flask
```bash
cd server
python app.py
```
### 3. Setup Frontend
Pastikan [NodeJS](https://nodejs.org/en) sudah terinstall, lalu install dependency. Di terminal baru, lakukan:
```bash
cd dashboard
npm install
```
Lalu jalankan server NextJS
```bash
npm run dev
```
klik tautan yang ada di terminal untuk mengakses dashboard

## Docs
Schema dari data yang tersimpan di database adalah sebagai berikut:
```json
{
  "data_raw": int,        // Nilai mentah dari sensor (ADC)
  "data_filtered": int,   // Hasil filter low-pass dari data mentah
  "bpm": int,             // Beats per minute (hasil estimasi)
  "created_at": ISODate   // Timestamp data dibuat
}
```

### `POST /data/publish_raw`
Mengirim data mentah dari sensor ke server.

**Request Body** (JSON):
```json
{
  "reading": 750
}
```

**Response** (201 Created):
```json
{
  "message": "Data added successfully",
  "id": "664e9cfb47d0a3e3b78df0e1"
}
```

---

### `GET /data`
Mengambil data terbaru dari database.

**Query Parameters**:
- `limit` (opsional, default = -1): jumlah maksimal data yang diambil. Jika tidak disertakan atau -1, akan mengambil semua data (tidak direkomendasikan).

**Example**:
```
GET /data?limit=50
```

**Response**:
```json
[
  {
    "_id": "664e9cfb47d0a3e3b78df0e1",
    "data_raw": 728,
    "data_filtered": 734,
    "bpm": 78,
    "created_at": "2024-06-25T17:32:05.115Z"
  },
  ...
]
```

---

### `GET /data/<id>`
Mengambil satu data berdasarkan ObjectId dari MongoDB.

**Example**:
```
GET /data/664e9cfb47d0a3e3b78df0e1
```

**Response (200 OK)**:
```json
{
  "_id": "664e9cfb47d0a3e3b78df0e1",
  "data_raw": 728,
  "data_filtered": 734,
  "bpm": 78,
  "created_at": "2024-06-25T17:32:05.115Z"
}
```

**Response (404 Not Found)**:
```json
{
  "error": "Data not found"
}
```

---

### `DELETE /data/clear`
Menghapus seluruh data dari database dan cache Backend.

**Example**:
```
DELETE /data/clear
```

**Response**:
```json
{
  "message": "100 records deleted"
}
```

## Authors
* 5024231010 - `Sebastian Adirian Nugraha`
* 5024231022 - `Michael`
* 5024231023 - `Edward Natasaputra`
* 5024231036 - `Farrel Ganendra`