import { BarList, Card } from "@tremor/react";
import React, { useEffect, useState } from "react";

const WorkHoursCalc = () => {
    const [workHours, setWorkHours] = useState([]);
    const [employeeData, setEmployeeData] = useState(null);
    const [calendarData, setCalendarData] = useState([]);
    const [selectedCardstr, setSelectedCardstr] = useState(null);
    const [employees, setEmployees] = useState([]);
    const [loading, setLoading] = useState(true);

    // Popup state
    const [isPopupOpen, setIsPopupOpen] = useState(false);
    const [newUserData, setNewUserData] = useState({
        cardstr: "",
        name: "",
        surname: "",
        phoneNumber: "",
        address: "",
        dateOfBirth: "",
    });

    useEffect(() => {
        const fetchEmployees = async () => {
            try {
                const response = await fetch(
                    "https://tzc3ef9y2g.execute-api.eu-north-1.amazonaws.com/fetch-items"
                );
                if (!response.ok) {
                    throw new Error("Failed to fetch employees.");
                }

                const data = await response.json();
                setEmployees(data);
            } catch (error) {
                console.error("Error fetching employees:", error);
            } finally {
                setLoading(false);
            }
        };
        fetchEmployees();
        console.log(employees);
    }, []);

    useEffect(() => {
        const fetchData = async () => {
            try {
                const response = await fetch(
                    "https://tzc3ef9y2g.execute-api.eu-north-1.amazonaws.com/getTable"
                );
                const data = await response.json();

                const calculatedWorkHours = calculateTotalWorkHours(data);
                setWorkHours(calculatedWorkHours);
            } catch (error) {
                console.error("Error fetching data:", error);
            }
        };
        fetchData();
    }, []);

    const calculateTotalWorkHours = (data) => {
        const groupByCard = data.reduce((acc, entry) => {
            if (!acc[entry.cardstr]) acc[entry.cardstr] = [];
            acc[entry.cardstr].push(entry);
            return acc;
        }, {});

        const workHoursArray = [];

        for (const cardstr in groupByCard) {
            const timestamps = groupByCard[cardstr].sort(
                (a, b) => a.timestamp - b.timestamp
            );

            let totalWorkHours = 0;

            for (let i = 0; i < timestamps.length - 1; i += 2) {
                const clockInTime = timestamps[i].timestamp;
                const clockOutTime = timestamps[i + 1].timestamp;

                const duration = (clockOutTime - clockInTime) / (60 * 60);
                totalWorkHours += duration;
            }

            workHoursArray.push({
                name: cardstr,
                value: parseFloat(totalWorkHours.toFixed(1)),
            });
        }

        return workHoursArray;
    };

    const handleCardstrClick = async (cardstr) => {
        try {
            const response = await fetch(
                `https://tzc3ef9y2g.execute-api.eu-north-1.amazonaws.com/employee-data?cardstr=${cardstr}`
            );
            const data = await response.json();

            const { employee, calendar } = data;

            setEmployeeData(employee);
            setCalendarData(calendar);
            setSelectedCardstr(cardstr);
        } catch (error) {
            console.error("Error fetching employee data:", error);
        }
    };

    const togglePopup = () => {
        setIsPopupOpen(!isPopupOpen);
    };

    const handleInputChange = (e) => {
        const { name, value } = e.target;
        setNewUserData({
            ...newUserData,
            [name]: value,
        });
    };

    const handleAddUser = async (e) => {
        e.preventDefault();
        try {
            const response = await fetch(
                "https://tzc3ef9y2g.execute-api.eu-north-1.amazonaws.com/add-item",
                {
                    method: "POST",
                    mode: "no-cors",
                    headers: {
                        "Content-Type": "application/json"
                    },
                    body: JSON.stringify(newUserData),
                },
                console.log(newUserData)

            );




            alert("User added.");

        } catch (error) {
            console.error("Error adding user:", error);
            alert("An error occurred. Please try again.");
        }
    };

    return (
        <div>
            {/* Work Hours Table */}
            <Card id="table" className="mx-auto max-w-lg">
                <h3 className="text-tremor-title text-tremor-content-strong dark:text-dark-tremor-content-strong font-medium">
                    Przepracowane godziny
                </h3>
                <p className="mt-4 text-tremor-default flex items-center justify-between text-tremor-content dark:text-dark-tremor-content">
                    <span>Pracownik</span>
                    <span>Liczba godzin</span>
                </p>
                <BarList data={workHours} className="mt-2" />
            </Card>

            {/* Add New User Button */}
            <button
                onClick={togglePopup}
                className="mt-6 mx-auto block bg-blue-500 text-white py-2 px-4 rounded hover:bg-blue-600"
            >
                Dodaj pracownika
            </button>

            {/* Add New User Popup */}
            {isPopupOpen && (
                <div className="fixed inset-0 bg-gray-800 bg-opacity-50 flex items-center justify-center z-50">
                    <div className="bg-white p-6 rounded shadow-lg max-w-md w-full z-60">
                        <h2 className="text-lg font-bold mb-4">Dodaj nowego pracownika</h2>
                        <form onSubmit={handleAddUser}>
                            <div className="mb-4">
                                <label className="block font-medium"> ID Karty</label>
                                <input
                                    type="text"
                                    name="cardstr"
                                    value={newUserData.cardstr}
                                    onChange={handleInputChange}
                                    className="w-full border border-gray-300 p-2 rounded"
                                    required
                                />
                            </div>
                            <div className="mb-4">
                                <label className="block font-medium">Imię</label>
                                <input
                                    type="text"
                                    name="name"
                                    value={newUserData.name}
                                    onChange={handleInputChange}
                                    className="w-full border border-gray-300 p-2 rounded"
                                    required
                                />
                            </div>
                            <div className="mb-4">
                                <label className="block font-medium">Nazwisko</label>
                                <input
                                    type="text"
                                    name="surname"
                                    value={newUserData.surname}
                                    onChange={handleInputChange}
                                    className="w-full border border-gray-300 p-2 rounded"
                                    required
                                />
                            </div>
                            <div className="mb-4">
                                <label className="block font-medium">Numer telefonu</label>
                                <input
                                    type="text"
                                    name="phoneNumber"
                                    value={newUserData.phoneNumber}
                                    onChange={handleInputChange}
                                    className="w-full border border-gray-300 p-2 rounded"
                                    required
                                />
                            </div>
                            <div className="mb-4">
                                <label className="block font-medium">Adres</label>
                                <input
                                    type="text"
                                    name="address"
                                    value={newUserData.address}
                                    onChange={handleInputChange}
                                    className="w-full border border-gray-300 p-2 rounded"
                                    required
                                />
                            </div>
                            <div className="mb-4">
                                <label className="block font-medium">Data urodzenia</label>
                                <input
                                    type="date"
                                    name="dateOfBirth"
                                    value={newUserData.dateOfBirth}
                                    onChange={handleInputChange}
                                    className="w-full border border-gray-300 p-2 rounded"
                                    required
                                />
                            </div>
                            <div className="flex justify-between">
                                <button
                                    type="submit"
                                    className="bg-green-500 text-white py-2 px-4 rounded hover:bg-green-600"
                                >
                                    Dodaj
                                </button>
                                <button
                                    type="button"
                                    onClick={togglePopup}
                                    className="bg-red-500 text-white py-2 px-4 rounded hover:bg-red-600"
                                >
                                    Anuluj
                                </button>
                            </div>
                        </form>
                    </div>
                </div>
            )}

            {/* List of Employees */}
            <Card className="mx-auto max-w-lg mt-6">
                <h3 className="text-tremor-title text-tremor-content-strong dark:text-dark-tremor-content-strong font-medium">
                    Lista Pracowników (RFID)
                </h3>
                <table className="table-auto mt-4 w-full border-collapse border dark:text-dark-tremor-content-strong font-medium">
                    <thead>
                        <tr>
                            <th className="border dark:text-dark-tremor-content-strong font-medium2">RFID</th>
                            <th className="border dark:text-dark-tremor-content-strong font-medium2">Imię</th>
                            <th className="border dark:text-dark-tremor-content-strong font-medium2">Nazwisko</th>
                        </tr>
                    </thead>
                    <tbody>
                        {employees.map((entry, index) => (
                            <tr key={index} className="hover:bg-black-100">
                                <td className="border dark:text-dark-tremor-content-strong font-medium"> <a className="text-blue-500 hover:text-blue-700" onClick={() => handleCardstrClick(entry.cardstr)}>{entry.cardstr} </a></td>
                                <td className="border dark:text-dark-tremor-content-strong font-medium">{entry.name}</td>
                                <td className="border dark:text-dark-tremor-content-strong font-medium">{entry.surname}</td>
                            </tr>
                        ))}
                    </tbody>
                </table>
            </Card>


            {/* Employee Info and Calendar */}
            {selectedCardstr && (
                <div className="mt-6">
                    <Card className="mx-auto max-w-lg">
                        <h3 className="text-tremor-title  dark:text-dark-tremor-content-strong font-medium">
                            Pracownik  - {selectedCardstr}
                        </h3>
                        {employeeData ? (
                            <table className="mt-4  dark:text-dark-tremor-content-strong font-medium">
                                <tbody>
                                    <tr>
                                        <th>Name:</th>
                                        <td>{employeeData.name}</td>
                                    </tr>
                                    <tr>
                                        <th>Surname:</th>
                                        <td>{employeeData.surname}</td>
                                    </tr>
                                    <tr>
                                        <th>Phone Number:</th>
                                        <td>{employeeData.phoneNumber}</td>
                                    </tr>
                                    <tr>
                                        <th>Address:</th>
                                        <td>{employeeData.address}</td>
                                    </tr>
                                    <tr>
                                        <th>Date of Birth:</th>
                                        <td>{employeeData.dateOfBirth}</td>
                                    </tr>
                                </tbody>
                            </table>
                        ) : (
                            <p>Loading employee data...</p>
                        )}
                    </Card>

                    <Card className="mx-auto max-w-lg mt-4">
                        <h3 className="text-tremor-title text-tremor-content-strong dark:text-dark-tremor-content-strong font-medium">
                            Logi {selectedCardstr}
                        </h3>
                        {calendarData.length > 0 ? (
                            <ul className="mt-4  dark:text-dark-tremor-content-strong font-medium">
                                {calendarData.map((event, index) => (
                                    <li key={index}>
                                        {event.date} {event.time} - {event.event_type}
                                    </li>
                                ))}
                            </ul>
                        ) : (
                            <p>No calendar data available.</p>
                        )}
                    </Card>
                </div>
            )}
        </div>
    );
};

export default WorkHoursCalc;
