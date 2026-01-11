import express from "express";
import db from "./config/database";

const app = express.Router();

interface active_server {
  name: string;
  address: string;
  port: number;
}

app.get("/active_server/:name", async (_req, res) => {
  const request: active_server[] = await db.manyOrNone(
    `
    SELECT
      a_s.game_id as id,
      a_s.ip_address as address,
      a_s.port as port
    FROM active_server a_s
    JOIN game g on g.id = a_s.game_id
    WHERE g.name = $1
    `,
    [_req.params.name],
  );
  res.json(request);
});

app.post("/active_server", async (_req, res) => {
  const id: number = (
    await db.query(
      `
      INSERT INTO active_server (game_id, ip_address, port)
      SELECT
        g.id,
        $1,
        $2
      FROM game g
      WHERE g.name = $3
      RETURNING id;
    `,
      [_req.body.ip, _req.body.port, _req.body.game_name],
    )
  )[0].id;
  res.json({ id });
});

app.delete("/active_server", async (_req, res) => {
  _req.body;
  await db.query(
    `
      DELETE FROM active_server
      WHERE id = $1
    `,
    [_req.body.id],
  );
  res.sendStatus(200);
});

export default app;
