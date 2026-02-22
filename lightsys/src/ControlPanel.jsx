import { db } from "./firebase";
import { onValue, ref, set } from "firebase/database";
import { useEffect, useState} from "react";

function ControlPanel(){
    const [ledState,setLedState] = useState(false);

    useEffect(() => {
        const ledRef = ref(db, "led");
        onValue(ledRef, (snapshot) =>{
        const data = snapshot.val();
        setLedState(data);
        });
    }, []);

    const toggleLed = () =>{
        const ledRef = ref(db, "led");
        set(ledRef, !ledState);
    };
    return(
        <div>
            <h2>Control Panel</h2>

        <button onClick={toggleLed}>
            Toggle LED
        </button>
        <h3>
        {ledState ? "🟢 LED este PORNIT" : "🔴 LED este OPRIT"}
        </h3>
    </div>
    //comment
    );
}

export default ControlPanel;