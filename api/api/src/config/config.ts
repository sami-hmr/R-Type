import dotenv from "dotenv";

dotenv.config();

interface Config {
  port: number;
  db_host: string;
  db_port: number;
  db_name: string;
  db_user: string;
  db_password: string;
}

const config: Config = {
  port: Number(process.env.API_PORT),
  db_host: process.env.DB_HOST,
  db_port: Number(process.env.DB_PORT),
  db_name: process.env.DB_NAME,
  db_user: process.env.DB_USER,
  db_password: process.env.DB_PASSWORD,
};

export default config;
