import express from "express";
import db from "./config/database";
import bcrypt from "bcrypt";

const app = express.Router();

async function hash_password(clear: string) {
  return bcrypt.hash(clear, 10);
}

app.post("/register", async (_req, res) => {
  const check: any[] = await db.query(
    `
    SELECT * FROM users WHERE identifier = $1;
  `,
    [_req.body.identifier],
  );

  if (check.length != 0) {
    return res.sendStatus(401);
  }

  const hashed_passowrd = hash_password(_req.body.password);

  try {
    const id = (
      await db.query(
        `
    INSERT INTO users (identifier, password)
    VALUES ($1, $2)
    RETURNING id;
    `,
        [_req.body.identifier, hashed_passowrd],
      )
    )[0].id;
    return res.json({ id });
  } catch (e) {
    res.sendStatus(401);
  }
});

app.post("/login", async (_req, res) => {
  try {
    console.log(_req.body);
    const hashed_passowrd = hash_password(_req.body.password);
    const id = (
      await db.oneOrNone(
        `
    SELECT id FROM users WHERE identifier = $1 AND password = $2;
  `,
        [_req.body.identifier, hashed_passowrd],
      )
    ).id;
    console.log(id);
    return res.json({ id });
  } catch (e) {
    res.sendStatus(401);
  }
});

export default app;
