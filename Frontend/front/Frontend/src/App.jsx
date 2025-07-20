import React from "react";
import Chart from "./components/Chart";
import LeftColumn from "./components/LeftColumn";
import Navbar from "./components/Navbar";
import RightColumn from "./components/RightColumn";
import Sidebar from "./components/Sidebar";
import EmployeeDetails from "./components/EmployeeDetails";
import { BrowserRouter as Router, Routes, Route } from "react-router-dom";
import '@aws-amplify/ui-react/styles.css';
import { Authenticator } from '@aws-amplify/ui-react';
import { Amplify } from 'aws-amplify';
import '@aws-amplify/ui-react/styles.css';
import IoTMessagePublisher from "./components/IoTMessagePublisher";

Amplify.configure({
  Auth: {
    Cognito: {
      userPoolId: "eu-north-1_IjZRbWhV3",
      userPoolClientId: "6r7650k3jc49qnb69lo2n6cm6i",
      identityPoolId: "eu-north-1:4fe42fbe-56e1-43ae-aeca-e4728987141e",
      loginWith: {
        email: true,
      },
      signUpVerificationMethod: "code",
      userAttributes: {
        email: {
          required: true,
        },
      },
      allowGuestAccess: true,
      passwordFormat: {
        minLength: 8,
        requireLowercase: true,
        requireUppercase: true,
        requireNumbers: true,
        requireSpecialCharacters: true,
      },
    },
  },
});

function App() {
  return (
    <Authenticator>
      {({ signOut, user }) => (
        <Router>
          <main className="flex">

            <Sidebar >
              <div className="relative">
                <button
                  onClick={signOut}
                  className=" left-4 bottom-3  bg-red-500 text-white py-2 px-4 rounded hover:bg-red-600"
                >
                  Wyloguj
                </button>
              </div>
            </Sidebar>
            <div className="flex flex-col flex-1 relative">
              <Navbar />
              <div className="grid md:grid-cols-1 grid-cols-1 w-full">
                <div className="w-full flex-col">
                  <Routes>
                    {/* Define routes */}
                    <Route path="/charts" element={<LeftColumn />} /> {/* Main Page */}
                    <Route path="/employees" element={<EmployeeDetails />} /> {/* Details Page */}
                    <Route path="/message" element={<IoTMessagePublisher />} /> {/* IoT Message Page */}
                  </Routes>
                </div>
              </div>
            </div>
          </main>
        </Router>
      )}
    </Authenticator>
  );
}

export default App;
