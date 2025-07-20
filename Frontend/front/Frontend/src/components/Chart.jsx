import React, { useEffect, useState } from "react";
import { AreaChart, LineChart } from "@tremor/react";

const mockData = [
    { Date: "Nov 11, 2023, 02:30 PM", "Light Level": 40.35 },
    { Date: "Nov 12, 2023, 03:15 PM", "Light Level": 50.12 }
];

const Chart = () => {
    const [chartData, setChartData] = useState([]);

    useEffect(() => {
        const fetchData = async () => {
            try {
                const response = await fetch("sdsdshttps://tzc3ef9y2g.execute-api.eu-north-1.amazonaws.com/getTelemetryTable");
                const data = await response.json();

                console.log("Fetched Data:", data);

                if (Array.isArray(data) && data.length > 0 && data[0].timestamp) {
                    const sortedData = data.sort((a, b) => a.timestamp - b.timestamp);

                    const formattedData = sortedData.map((item) => ({
                        Date: new Date(item.timestamp * 1000).toLocaleDateString("en-US", {
                            year: "numeric",
                            month: "short",
                            day: "numeric",
                            hour: "2-digit",
                            minute: "2-digit",
                        }),
                        "Light Level": item.light_level,
                    }));

                    console.log("Formatted Data for Chart:", formattedData);
                    setChartData(formattedData);
                } else {
                    console.error("Unexpected data format:", data);
                }
            } catch (error) {
                console.error("Error fetching data", error);
            }
        };

        fetchData();
    }, []);



    return (
        <div className="p-4 bg-white rounded shadow">
            <h2 className="text-xl font-semibold mb-4">Light Level Over Time</h2>
            <AreaChart
                data={chartData}
                index="Date"
                categories={["Light Level"]}
                colors={["blue"]}
                valueFormatter={(value) => `${value} lux`}
                marginTop="mt-4"
                height="h-72"

            />
        </div>
    );
};

export default Chart;
