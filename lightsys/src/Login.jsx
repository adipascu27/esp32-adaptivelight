import { useState } from "react";
import { signInWithEmailAndPassword } from "firebase/auth";
import { auth } from "./firebase";
import "./Login.css";

function Login({ setUser }) {
  const [email, setEmail] = useState("");
  const [password, setPassword] = useState("");
  const [error, setError] = useState("");
  const [loading, setLoading] = useState(false);

  const login = async () => {
    if (!email || !password) {
      setError("Please enter your email and password.");
      return;
    }
    setError("");
    setLoading(true);
    try {
      const userCredential = await signInWithEmailAndPassword(auth, email, password);
      setUser(userCredential.user);
    } catch (err) {
      setError("Invalid email or password. Please try again.");
    } finally {
      setLoading(false);
    }
  };

  const handleKeyDown = (e) => {
    if (e.key === "Enter") login();
  };

  return (
    <div className="login-root">
      <div className="login-card">
        <div className="login-logo">
          <span className="login-logo-dot" />
          <span className="login-logo-text">LightSys</span>
        </div>

        <h1 className="login-heading">Sign in</h1>
        <p className="login-sub">Access your lighting control panel</p>

        <div className="login-fields">
          <div className="login-field">
            <label className="login-label" htmlFor="email">Email</label>
            <input
              id="email"
              className="login-input"
              type="email"
              placeholder="you@example.com"
              value={email}
              onChange={(e) => setEmail(e.target.value)}
              onKeyDown={handleKeyDown}
              autoComplete="email"
            />
          </div>

          <div className="login-field">
            <label className="login-label" htmlFor="password">Password</label>
            <input
              id="password"
              className="login-input"
              type="password"
              placeholder="••••••••"
              value={password}
              onChange={(e) => setPassword(e.target.value)}
              onKeyDown={handleKeyDown}
              autoComplete="current-password"
            />
          </div>
        </div>

        {error && <p className="login-error">{error}</p>}

        <button
          className="login-btn"
          onClick={login}
          disabled={loading}
        >
          {loading ? (
            <span className="login-spinner" />
          ) : (
            "Sign in"
          )}
        </button>
      </div>
    </div>
  );
}

export default Login;
