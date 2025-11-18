import express from "express";
import path from "path";

const app = express();

app.use(express.static(path.resolve("./"), {
    setHeaders: (res, path) => {
        res.set("Cross-Origin-Opener-Policy", "same-origin");
        res.set("Cross-Origin-Embedder-Policy", "require-corp");
        if (path.endsWith(".wasm")) {
            res.set("Content-Type", "application/wasm");
        }
    }
}));

app.use(express.static(path.resolve("../")));

app.listen(3000, () => console.log("Listening on http://localhost:3000"));
