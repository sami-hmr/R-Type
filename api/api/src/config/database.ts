import pgPromise from "pg-promise";
import config from "./config";

const pgp = pgPromise();

const db = pgp({
  host: config.db_host,
  port: config.db_port || 5432,
  database: config.db_name,
  user: config.db_user,
  password: config.db_password,
});

export default db;
