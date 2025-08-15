-- ============================================================
--   REQUEST: UPDATE                
-- ============================================================

UPDATE livraison
SET statut_livraison = 'NEW_STATUT'
WHERE livraison_id = 'LIVRAISON_ID'
RETURNING *;

-- ============================================================
--   RESPONSE: INSERTION
-- ============================================================

INSERT INTO 'TABLE' ('COLUMN1', 'COLUMN2', 'COLUMN3',...) VALUES ('VALUE1', 'VALUE2', 'VALUE3',...);


-- ============================================================
--   RESPONSE: DELETE
-- ============================================================

DELETE FROM 'TABLE' WHERE table_id = 'TABLE_ID';