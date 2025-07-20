import React from 'react'
import { ArrowUpIcon, ChartBarIcon, ChatBubbleLeftIcon, HomeIcon, TableCellsIcon } from '@heroicons/react/24/solid'
import { NavLink } from 'react-router'

const Sidebar = ({ children }) => {
    return (
        <div className="bg-slate-800 flex-none w-14 sm:w-20 h-screen">
            <div className="h-20 items-center flex">
                <HomeIcon width={40} className="text-gray-300 left-3 sm:left-6 fixed" />
            </div>
            <div className="fixed left-3 sm:left-6 top-[100px]">
                <a >
                    <NavLink to="/charts">
                        <ChartBarIcon width={40} className="bg-gray-600 p-2 rounded-lg mb-4 text-gray-300" />
                    </NavLink>
                </a>
                <a >
                    <NavLink to="/employees">
                        <TableCellsIcon width={40} className="bg-gray-600 p-2 rounded-lg mb-4 text-gray-300" />
                    </NavLink>
                </a>
                <a>
                    <NavLink to="/message">
                        <ChatBubbleLeftIcon width={40} className="bg-gray-600 p-2 rounded-lg mb-4 text-gray-300" />
                    </NavLink>
                </a>
            </div>
            <div className="fixed bottom-4 left-3 sm:left-6">
                <a href="#top">

                    <ArrowUpIcon width={40} className="bg-gray-600 p-2 rounded-lg mb-4 text-gray-300" />
                </a>
            </div>
            <div className="fixed bottom-20 left-3">{children}</div>
        </div>
    )
}

export default Sidebar