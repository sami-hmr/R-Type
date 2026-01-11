import express from "express";
import config from "./config/config";
import active_server from "./active_server";
import game from "./game";
const app = express();

app.set("trust proxy", true);
app.use(express.json());

app.get("/", (_req, res) => {
  res.send("Hello World!");
  console.log("Response sent");
});

app.use(active_server);
app.use(game);

app.listen(config.port, () => {
  console.log(`Example app listening on port ${config.port}`);
});
