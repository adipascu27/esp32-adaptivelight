import { db } from "./firebase";
import { onValue, ref, set } from "firebase/database";
import { useEffect, useState } from "react";
import { PATHS } from "./paths";
import { SketchPicker } from "react-color";
import "./ControlPanel.css";

function ControlPanel() {
  const [ledState, setLedState] = useState(false);
  const [mode, setMode] = useState(0);
  const [brightness, setBrightness] = useState(0);
  const [color, setColor] = useState({ r: 255, g: 0, b: 0 });
  const [showPicker, setShowPicker] = useState(false);

  const modeRef = ref(db, "mode");
  const brightnessRef = ref(db, "manual/brightness");

  useEffect(() => {
    const ledRef = ref(db, PATHS.LED);
    onValue(ledRef, (snapshot) => {
      const data = snapshot.val();
      setLedState(data);
    });
    onValue(modeRef, (snapshot) => {
      if (snapshot.exists()) setMode(snapshot.val());
    });
    onValue(brightnessRef, (snapshot) => {
      if (snapshot.exists()) setBrightness(snapshot.val());
    });
  }, []);

  const handleModeChange = (newMode) => set(modeRef, newMode);
  const changeBrightness = (value) => set(brightnessRef, value);
  const toggleLed = () => {
    const ledRef = ref(db, PATHS.LED);
    set(ledRef, !ledState);
  };
  const sendColorToFirebase = (rgb) => {
    set(ref(db, "manual/color"), { r: rgb.r, g: rgb.g, b: rgb.b });
  };

  const rgbToHex = ({ r, g, b }) =>
    "#" + [r, g, b].map((v) => v.toString(16).padStart(2, "0")).join("");

  return (
    <div className="cp-root">
      <header className="cp-header">
        <div className="cp-header-left">
          <span className="cp-logo-dot" />
          <span className="cp-title">LightSys</span>
        </div>
        <div className={`cp-status-chip ${ledState ? "on" : "off"}`}>
          <span className="cp-status-dot" />
          {ledState ? "Online" : "Offline"}
        </div>
      </header>

      <main className="cp-main">
        {/* Mode selector */}
        <section className="cp-card">
          <p className="cp-label">Operating mode</p>
          <div className="cp-mode-toggle">
            <button
              className={`cp-mode-btn ${mode === 0 ? "active" : ""}`}
              onClick={() => handleModeChange(0)}
            >
              <span className="cp-mode-icon">&#9680;</span>
              Manual
            </button>
            <button
              className={`cp-mode-btn ${mode === 1 ? "active" : ""}`}
              onClick={() => handleModeChange(1)}
            >
              <span className="cp-mode-icon">&#10026;</span>
              Automatic
            </button>
          </div>
        </section>

        {/* Manual controls */}
        {mode === 0 && (
          <>
            {/* LED toggle */}
            <section className="cp-card cp-led-card">
              <div className="cp-led-info">
                <p className="cp-label">LED strip</p>
                <p className={`cp-led-state ${ledState ? "on" : "off"}`}>
                  {ledState ? "Powered on" : "Powered off"}
                </p>
              </div>
              <button
                className={`cp-toggle ${ledState ? "on" : "off"}`}
                onClick={toggleLed}
                aria-label="Toggle LED"
              >
                <span className="cp-toggle-thumb" />
              </button>
            </section>

            {/* Brightness */}
            <section className="cp-card">
              <div className="cp-row-between">
                <p className="cp-label">Brightness</p>
                <span className="cp-value">{brightness}%</span>
              </div>
              <input
                className="cp-slider"
                type="range"
                min="0"
                max="100"
                step="1"
                value={brightness}
                onChange={(e) => changeBrightness(Number(e.target.value))}
              />
              <div className="cp-slider-marks">
                <span>0%</span>
                <span>50%</span>
                <span>100%</span>
              </div>
            </section>

            {/* Color */}
            <section className="cp-card">
              <div className="cp-row-between">
                <p className="cp-label">Color</p>
                <button
                  className="cp-color-preview-btn"
                  style={{ background: rgbToHex(color) }}
                  onClick={() => setShowPicker((v) => !v)}
                  aria-label="Open color picker"
                />
              </div>
              {showPicker && (
                <div className="cp-picker-wrap">
                  <SketchPicker
                    color={color}
                    disableAlpha
                    onChangeComplete={(newColor) => {
                      const rgb = newColor.rgb;
                      setColor(rgb);
                      sendColorToFirebase(rgb);
                    }}
                  />
                </div>
              )}
            </section>
          </>
        )}

        {/* Automatic mode */}
        {mode === 1 && (
          <section className="cp-card cp-auto-card">
            <div className="cp-auto-icon">&#9685;</div>
            <p className="cp-auto-title">Sensor control active</p>
            <p className="cp-auto-sub">
              The system adjusts lighting automatically based on ambient
              luminosity and motion detection. No manual input required.
            </p>
          </section>
        )}
      </main>
    </div>
  );
}

export default ControlPanel;
