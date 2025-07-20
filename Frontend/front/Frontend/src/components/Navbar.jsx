import { ChatBubbleLeftEllipsisIcon, MagnifyingGlassIcon } from "@heroicons/react/24/solid"
import { TextInput } from "@tremor/react"
import React from 'react'

const Navbar = () => {
    return (
        <div id="top" className="p-2">

            <div>
                <TextInput placeholder="" icon={ChatBubbleLeftEllipsisIcon} />
            </div>
        </div>
    )
}

export default Navbar