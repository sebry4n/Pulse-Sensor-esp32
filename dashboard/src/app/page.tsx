// app/page.tsx
'use client';

import { useEffect, useState, useRef } from 'react';
import { Line } from 'react-chartjs-2';
import {
  Chart as ChartJS,
  LineElement,
  CategoryScale,
  LinearScale,
  PointElement,
  Legend,
  Tooltip,
} from 'chart.js';
import { Card, CardContent } from '@/components/ui/card';
import { Button } from '@/components/ui/button';

ChartJS.register(LineElement, CategoryScale, LinearScale, PointElement, Legend, Tooltip);

const fetchData = async () => {
  const res = await fetch('http://192.168.58.12:5000/data?limit=100');
  if (!res.ok) throw new Error('Failed to fetch data');
  const data = await res.json();
  console.log('Fetched data:', data); // Log the fetched data for debugging
  return data;
};

const clearData = async () => {
  const res = await fetch('http://192.168.58.12:5000/data/clear', { method: 'DELETE' });
  if (!res.ok) throw new Error('Failed to clear data');
};

export default function Home() {
  const [data, setData] = useState<any[]>([]);
  const [loading, setLoading] = useState(false);
  const intervalRef = useRef<NodeJS.Timeout | null>(null);

  const loadData = async () => {
    try {
      const res = await fetchData();
      setData(res);
    } catch (err) {
      console.error(err);
    }
  };

  useEffect(() => {
    loadData();
    intervalRef.current = setInterval(() => {
      loadData();
    }, 100); // faster refresh

    return () => {
      if (intervalRef.current) clearInterval(intervalRef.current);
    };
  }, []);

  const handleClear = async () => {
    try {
      await clearData();
      setData([]);
    } catch (err) {
      console.error(err);
    }
  };

  // Use all data points, but control display through chart options
  let rawData = data.map((entry) => entry.data_raw);
  let filteredData = data.map((entry) => entry.data_filtered);

  if (rawData.length < 100) {
    rawData = rawData.toReversed();
    filteredData = filteredData.toReversed();
  }

  const chartData = {
    labels: Array.from({ length: 100 }, (_, i) => i + 1), // Start labels at 1
    datasets: [
      {
        label: 'Raw Data',
        data: rawData,
        borderColor: 'rgba(255, 99, 132, 1)',
        backgroundColor: 'rgba(255, 99, 132, 0.2)',
        fill: false,
        pointRadius: 0, // Remove points for cleaner look
      },
      {
        label: 'Filtered Data',
        data: filteredData,
        borderColor: 'rgba(54, 162, 235, 1)',
        backgroundColor: 'rgba(54, 162, 235, 0.2)',
        fill: false,
        pointRadius: 0, // Remove points for cleaner look
      },
    ],
  };

  const chartOptions = {
    responsive: true,
    maintainAspectRatio: false,
    animation: {
      duration: 0,
    },
    plugins: {
      legend: {
        position: 'top' as const,
      },
    },
    scales: {
      x: {
        display: true,
        title: {
          display: true,
          text: 'Data Points',
        },
        ticks: {
          autoSkip: true,
          maxTicksLimit: 20,
        },
        // This makes the chart grow from left to right
        // and only start scrolling when we have more than 100 points
        min: data.length > 100 ? data.length - 100 : 0,
        max: data.length > 100 ? data.length : 100,
      },
      y: {
        beginAtZero: true,
        min: 0,
        max: 160,
      },
    },
    elements: {
      line: {
        tension: 0.1, // Reduced tension for more accurate representation
      },
    },
  };

  const latestBPM = data.length > 0 ? data[data.length - 1].bpm : 0;

  return (
    <main className="min-h-screen bg-gray-100 flex flex-row items-center justify-center p-4">
      <Card className="w-full max-w-4xl shadow-md">
        <CardContent className="space-y-4 p-6">
          <h1 className="text-2xl font-bold">Pulse Sensor Dashboard</h1>
          <div className="flex flex-col md:flex-row md:items-center md:justify-between gap-2">
            <p className="text-lg font-semibold">Latest BPM: {latestBPM}</p>
            <div className="flex gap-2">
              <Button onClick={loadData} disabled={loading}>
                {loading ? "Loading..." : "Reload"}
              </Button>
              <Button variant="destructive" onClick={handleClear}>
                Clear Data
              </Button>
            </div>
          </div>
          <div className="h-96 w-full">
            <Line data={chartData} options={chartOptions} />
          </div>
          <div className="">
            <p className="font-semibold mb-2">
              Panduan Umum Detak Jantung per Menit (BPM)
            </p>
            <ul className="list-disc list-inside">
              <li>Bayi baru lahir (0–3 bulan): 100–150 BPM</li>
              <li>Bayi (3–6 bulan): 90–120 BPM</li>
              <li>Balita (6–12 bulan): 80–120 BPM</li>
              <li>Anak-anak (1–10 tahun): 70–110 BPM</li>
              <li>
                Anak-anak di atas 10 tahun dan Dewasa (termasuk lansia): 60–100
              </li>
              <li>
                Atlet terlatih: 40–60 BPM
              </li>
              <li>
                Bertemu Furina: tak hingga
              </li>
            </ul>
          </div>
        </CardContent>
      </Card>
    </main>
  );
}