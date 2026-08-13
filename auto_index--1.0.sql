
CREATE TABLE IF NOT EXISTS aidx_queries (
    tablename TEXT,
    colname TEXT,  -- <- rename this
    cost DOUBLE PRECISION,
    benefit DOUBLE PRECISION,
    PRIMARY KEY (tablename, colname)
);