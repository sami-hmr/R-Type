import express from "express";
import db from "./config/database";

const app = express.Router();

app.post("/game", async (_req, res) => {
  await db.query("INSERT INTO game (name) VALUES ($1)", [_req.body.name]);
  res.sendStatus(200);
});

app.delete("/game", async (_req, res) => {
  await db.query("DELETE from game WHERE name = $1", [_req.body.name]);
  res.sendStatus(200);
});

export default app;
