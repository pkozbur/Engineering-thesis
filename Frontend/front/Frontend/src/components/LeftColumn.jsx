import React, { useEffect, useState } from "react";
import { AreaChart } from "@tremor/react";
import WorkHoursCalc from "./WorkHoursCalc";
import { Link } from "react-router-dom";
import IoTMessagePublisher from "./IoTMessagePublisher";

const LeftColumn = () => {
    const [chartData, setChartData] = useState([]);

    useEffect(() => {
        const fetchData = async () => {
            try {
                const response = await fetch(
                    "https://tzc3ef9y2g.execute-api.eu-north-1.amazonaws.com/getTelemetryTable"
                );
                const data = await response.json();

                console.log("Fetched Data:", data);
                if (Array.isArray(data) && data.length > 0 && data[0].timestamp) {
                    const sortedData = data.sort((a, b) => a.timestamp - b.timestamp);
                    const formattedData = sortedData.map((item) => ({
                        Date: new Date(item.timestamp * 1000).toLocaleString("en-US", {
                            year: "numeric",
                            month: "short",
                            day: "numeric",
                            hour: "2-digit",
                            minute: "2-digit",
                        }),
                        "Poziom światła": item.light_level,
                        "Ciśnienie": item.pressure,
                        "Liczba ludzi": item.people_count,
                        "Temperatura": item.temperature,
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
        <div id="charts" className="grid grid-cols-1 md:grid-cols-2 gap-6 p-6">
            <div className="p-6 bg-white rounded shadow">
                <h2 className="text-xl font-semibold mb-6">Liczba osób w czasie</h2>
                <AreaChart
                    data={chartData}
                    index="Date"
                    categories={["Liczba ludzi"]}
                    colors={["indigo"]}
                    valueFormatter={(value) => `${value} `}
                    marginTop="mt-6"
                    height="h-72"
                />
            </div>
            <div className="p-6 bg-white rounded shadow">
                <h2 className="text-xl font-semibold mb-6">Wartość temperatury w czasie</h2>
                <AreaChart
                    data={chartData}
                    index="Date"
                    categories={["Temperatura"]}
                    colors={["cyan"]}
                    valueFormatter={(value) => `${value} °C`}
                    marginTop="mt-4"
                    height="h-72"
                />
            </div>
            <div className="p-6 bg-white rounded shadow">
                <h2 className="text-xl font-semibold mb-6">Wartość ciśnienia w czasie</h2>
                <AreaChart
                    data={chartData}
                    index="Date"
                    categories={["Ciśnienie"]}
                    colors={["blue"]}
                    valueFormatter={(value) => `${value} hPa`}
                    marginTop="mt-4"
                    height="h-72"
                />
            </div>
            <div className="p-6 bg-white rounded shadow">
                <h2 className="text-xl font-semibold mb-6">Poziom światła w czasie</h2>
                <AreaChart
                    data={chartData}
                    index="Date"
                    categories={["Poziom światła"]}
                    colors={["red"]}
                    valueFormatter={(value) => `${value} Światło`}
                    marginTop="mt-4"
                    height="h-72"
                />
            </div>


        </div>
    );
};

export default LeftColumn;
