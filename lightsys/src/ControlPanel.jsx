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
  const [colorMode, setColorMode] = useState(0);
  const [animSpeed, setAnimSpeed] = useState(50);
  const colorModeRef = ref(db, "manual/colorMode");
  const animSpeedRef = ref(db, "manual/animSpeed");
  const modeRef = ref(db, "mode");
  const brightnessRef = ref(db, "manual/brightness");
  const [lux, setLux] = useState(0);
  const [motion, setMotion] = useState(false);
  const [autoLedOn, setAutoLedOn] = useState(false);
  const [remaining, setRemaining] = useState(0);
  const [isDark, setIsDark] = useState(false);
  const [lastMotion, setLastMotion] = useState(null);
  const handleBrightnessChange = (e) => {
    setBrightness(Number(e.target.value));
  };

  const handleAnimSpeedChange = (e) => {
    setAnimSpeed(Number(e.target.value));
  };


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
    onValue(colorModeRef, (snapshot) => {
    if (snapshot.exists()) setColorMode(snapshot.val());
    });
    onValue(animSpeedRef, (snapshot) => {
        if (snapshot.exists()) setAnimSpeed(snapshot.val());
    });
    onValue(ref(db, "status/lux"), (snap) => {
        if (snap.exists()) setLux(snap.val());
    });
    onValue(ref(db, "status/motion"), (snap) => {
        if (snap.exists()) setMotion(snap.val());
    });
    onValue(ref(db, "status/ledState"), (snap) => {
        if (snap.exists()) setAutoLedOn(snap.val());
    });
    onValue(ref(db, "status/remaining"), (snap) => {
        if (snap.exists()) setRemaining(snap.val());
    });
    onValue(ref(db, "status/isDark"), (snap) => {
        if (snap.exists()) setIsDark(snap.val());
    });
    onValue(ref(db, "status/lastMotion"), (snap) => {
        if (snap.exists()) setLastMotion(new Date(snap.val() * 1000));
    });
  }, []);

  const formatCountdown = (secs) => {
    const m = Math.floor(secs / 60).toString().padStart(2, "0");
    const s = (secs % 60).toString().padStart(2, "0");
    return `${m}:${s}`;
  };

  const formatLastMotion = (date) => {
    if (!date) return "—";
    return date.toLocaleTimeString("ro-RO", {
        hour: "2-digit", minute: "2-digit", second: "2-digit"
    });
  };

  const getLuxInfo = (lux) => {
    if (lux < 15)  return { icon: "🌕", label: "Noapte",       color: "#4a4a6a" };
    if (lux < 25)  return { icon: "🌙", label: "Lună și stele", color: "#5a5a8a" };
    if (lux < 40)  return { icon: "🌅", label: "Răsărit/apus", color: "#c47a3a" };
    if (lux < 60)  return { icon: "⛅", label: "Parțial înnorat", color: "#a0a040" };
    return         { icon: "☀️", label: "Lumină puternică", color: "#d4a020" };
  };

  const changeColorMode = (val) => set(colorModeRef, val);
  const changeAnimSpeed = (val) => set(animSpeedRef, animSpeed);
  const handleModeChange = (newMode) => set(modeRef, newMode);
  const changeBrightness = (value) => set(brightnessRef, brightness);
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
                onChange={handleBrightnessChange}
                onMouseUp={changeBrightness}
                onTouchEnd={changeBrightness}
              />
              <div className="cp-slider-marks">
                <span>0%</span>
                <span>50%</span>
                <span>100%</span>
              </div>
            </section>

            {/* Color */}
            {colorMode !== 2 && colorMode !== 3 && colorMode !== 4 && (
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
            )}
            


            <section className="cp-card">
            <p className="cp-label">Color Mode</p>
            <div className="cp-colormode-grid">
                {[
                    { id: 0, label: "Static",   icon: "⬛" },
                    { id: 1, label: "Breathing", icon: "🫧" },
                    { id: 2, label: "Strobe",   icon: "⚡" },
                    { id: 3, label: "Smooth",   icon: "🌈" },
                    { id: 4, label: "Wave",     icon: "〰️" },
                ].map((m) => (
                    <button
                        key={m.id}
                        className={`cp-colormode-btn ${colorMode === m.id ? "active" : ""}`}
                        onClick={() => changeColorMode(m.id)}
                    >
                        <span className="cp-colormode-icon">{m.icon}</span>
                        <span>{m.label}</span>
                    </button>
                ))}
            </div>
        </section>

          {/* Anim Speed — ascuns pentru Static */}
          {colorMode !== 0 && (
              <section className="cp-card">
                  <div className="cp-row-between">
                      <p className="cp-label">Animation Speed</p>
                      <span className="cp-value">{animSpeed}%</span>
                  </div>
                  <input
                      className="cp-slider"
                      type="range"
                      min="0"
                      max="100"
                      value={animSpeed}
                      onChange={handleAnimSpeedChange}
                      onMouseUp={changeAnimSpeed}
                      onTouchEnd={changeAnimSpeed}
                  />
                  <div className="cp-slider-marks">
                      <span>Slow</span>
                      <span>Fast</span>
                  </div>
              </section>
          )}
          </>
        )}

        {/* Automatic mode */}
        {mode === 1 && (() => {
    const luxInfo = getLuxInfo(lux);
    const luxPercent = Math.min(lux / 100 * 100, 100);

    return (
        <div className="auto-dashboard">

            {/* Lux card — vizualizare */}
            <section className="cp-card auto-lux-card">
                <p className="cp-label">Iluminare ambientală</p>
                <div className="auto-lux-visual">
                    <div
                        className="auto-lux-icon"
                        style={{ color: luxInfo.color }}
                    >
                        {luxInfo.icon}
                    </div>
                    <div className="auto-lux-right">
                        <div className="auto-lux-value">
                            {lux.toFixed(1)}
                            <span className="auto-lux-unit">lux</span>
                        </div>
                        <div className="auto-lux-label"
                             style={{ color: luxInfo.color }}>
                            {luxInfo.label}
                        </div>
                        <div className="auto-lux-bar-track">
                            <div
                                className="auto-lux-bar-fill"
                                style={{
                                    width: `${luxPercent}%`,
                                    background: luxInfo.color
                                }}
                            />
                        </div>
                    </div>
                </div>
                <div className="auto-lux-thresholds">
                    <span>Prag întuneric: &lt;40 lux</span>
                    <span>Prag luminos: &gt;60 lux</span>
                </div>
            </section>

            {/* Status grid */}
            <div className="auto-status-grid">
                {/* PIR */}
                <section className="cp-card auto-stat-card">
                    <p className="cp-label">Senzor PIR</p>
                    <div className={`auto-stat-indicator ${motion ? "active" : ""}`}>
                        <span className="auto-stat-dot" />
                    </div>
                    <p className="auto-stat-label">
                        {motion ? "Mișcare detectată" : "Fără mișcare"}
                    </p>
                </section>

                {/* LED state */}
                <section className="cp-card auto-stat-card">
                    <p className="cp-label">Banda LED</p>
                    <div className={`auto-stat-indicator ${autoLedOn ? "led-on" : ""}`}>
                        <span className="auto-stat-dot" />
                    </div>
                    <p className="auto-stat-label">
                        {autoLedOn ? "Pornită" : "Oprită"}
                    </p>
                </section>
            </div>

            {/* Countdown */}
            <section className="cp-card auto-countdown-card">
                <p className="cp-label">Stingere automată</p>
                {autoLedOn ? (
                    <>
                        <div className="auto-countdown-timer">
                            {formatCountdown(remaining)}
                        </div>
                        <div className="auto-countdown-bar-track">
                            <div
                                className="auto-countdown-bar-fill"
                                style={{
                                    width: `${(remaining / (15 * 60)) * 100}%`
                                }}
                            />
                        </div>
                        <p className="auto-countdown-sub">
                            Timerul se resetează la fiecare detecție PIR
                        </p>
                    </>
                ) : (
                    <p className="auto-countdown-inactive">
                        — banda LED este oprită —
                    </p>
                )}
            </section>

            {/* Meta info */}
            <section className="cp-card auto-meta-card">
                <div className="auto-meta-row">
                    <span className="auto-meta-key">Mod curent</span>
                    <span className="auto-meta-val">
                        {isDark ? "🌙 Monitorizare activă" : "☀️ Lumină suficientă"}
                    </span>
                </div>
                <div className="auto-meta-row">
                    <span className="auto-meta-key">Ultima detecție</span>
                    <span className="auto-meta-val">
                        {formatLastMotion(lastMotion)}
                    </span>
                </div>
            </section>

        </div>
        );
    })()}
      </main>
    </div>
  );
}

export default ControlPanel;
