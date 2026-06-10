import { db } from "./firebase";
import { onValue, ref, set } from "firebase/database";
import { useEffect, useState} from "react";
import { PATHS } from "./paths";
import { SketchPicker } from "react-color";

function ControlPanel(){
    const [ledState,setLedState] = useState(false);
    const [mode, setMode] = useState(0);
    const [brightness, setBrightness] = useState(0);
    const [color, setColor] = useState({ r: 255, g: 0, b: 0 });
    const modeRef = ref(db, "mode");
    const brightnessRef = ref(db, "manual/brightness");
    useEffect(() => {
        const ledRef = ref(db, PATHS.LED);
        onValue(ledRef, (snapshot) =>{
            const data = snapshot.val();
            setLedState(data);
        });

        onValue(modeRef, (snapshot) =>{
            if (snapshot.exists()){
                setMode(snapshot.val());
            }
        });
        onValue(brightnessRef, (snapshot) => {
            if (snapshot.exists()) {
                setBrightness(snapshot.val());
            }
        });
    }, []);
    const handleModeChange = (newMode) => {
        set(modeRef, newMode);
    };
    const changeBrightness = (value) => {
        set(brightnessRef, value);
    };
    const toggleLed = () =>{
        const ledRef = ref(db, PATHS.LED);
        set(ledRef, !ledState);
    };
    const sendColorToFirebase = (rgb) => {
        set(ref(db, "manual/color"), {
            r: rgb.r,
            g: rgb.g,
            b: rgb.b
        });
    };
    return(
        <div>
            <h2>Control Panel</h2>
            <h2>Mode: {mode ? "Automatic" : "Manual"}</h2>

            <button onClick={() => handleModeChange(0)}>
                Manual
            </button>

            <button onClick={() => handleModeChange(1)}>
                Auto
            </button>
            {mode === 0 && (
            <div>
                <button onClick={toggleLed}>
                    Toggle LED
                </button>
                <h3>
                    {ledState ? "🟢 LED este PORNIT" : "🔴 LED este OPRIT"}
                </h3>
                <input
                    type="range"
                    min="0"
                    max="100"
                    value={brightness}
                    onChange={(e) => changeBrightness(Number(e.target.value))}
                />

                <p>Brightness: {brightness}%</p>
                <SketchPicker
                    color={color}
                    onChangeComplete={(newColor) => {
                        const rgb = newColor.rgb;
                        setColor(rgb);
                        sendColorToFirebase(rgb);
                    }}
                />
            </div>
    )}

    {mode === 1 && (
        <p>Automatic mode active (controlled by sensors)</p>
    )}
    </div>
    //comment
    );
}

export default ControlPanel;