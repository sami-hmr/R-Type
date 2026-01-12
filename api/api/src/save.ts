import express from "express";
import db from "./config/database";

const app = express.Router();

app.post("/get_save", async (_req, res) => {
  res.set("id", _req.body.id);

  console.log(_req.body)
  const r: any[] = await db.query(`
    SELECT
      saves.save as save
    FROM saves
    JOIN game g ON saves.game_id = g.id
    WHERE saves.user_id = $1 AND g.name = $2
    `, [_req.body.id, _req.body.game]);

  if (r.length == 0) {
    console.log("not found");
    return res.send(`NOT FOUND`);
  }
  res.set('Content-Type', 'application/octet-stream');
  return res.send(r[0].save);
});

app.use("/save", express.raw({ type: "application/octet-stream" }));

app.post("/save", async (_req, res) => {
  console.log(_req.body)
  try {
    const r: any[] = await db.query(`
    INSERT INTO saves (user_id, game_id, save)
    VALUES (
      $1,
      (SELECT id FROM game WHERE name = $2),
      $3
    )
    ON CONFLICT (user_id, game_id)
    DO UPDATE SET save = EXCLUDED.save
    `, [_req.header("user-id"), _req.header("game-name"), _req.body]);
    res.sendStatus(200);
  } catch (e) {
    res.sendStatus(401);
  }
});


export default app;
