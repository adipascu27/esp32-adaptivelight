import logo from './logo.svg';
import './App.css';
import { db } from "./firebase";
import { ref, set } from "firebase/database";
import Login from "./Login";
import { auth } from "./firebase";
import ControlPanel from "./ControlPanel";
import { useEffect, useState } from "react";
import { onAuthStateChanged } from "firebase/auth";

function App() {
  const [user, setUser] = useState(null);

  useEffect(() => {
    onAuthStateChanged(auth, (currentUser) => {
      setUser(currentUser);
    });
  }, []);

  return (
    <div>
      {user ? <ControlPanel /> : <Login setUser={setUser} />}
    </div>
  );
}

export default App;