CREATE TABLE IF NOT EXISTS request_logs (
    id SERIAL PRIMARY KEY,
    timestamp TIMESTAMPTZ DEFAULT NOW(),
    method TEXT,
    origin_path TEXT,
    target_host TEXT,
    forward_path TEXT,
    status_code INTEGER,
    latency_ms INTEGER,
    response_size BIGINT,
    headers_injected BOOLEAN
);