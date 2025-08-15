-- ============================================================
--   Nom de la base   :  LOGISTIQUE                                
--   Nom de SGBD      :  PostgreSQL                   
--   Date de creation :  25/10/2024                    
-- ============================================================

CREATE EXTENSION IF NOT EXISTS pgcrypto;

-- ============================================================
--   Table : APPARTENIR                                            
-- ============================================================
CREATE TABLE APPARTENIR (
  PRIMARY KEY (modele_id, camion_id),
  modele_id                 UUID                            NOT NULL,
  camion_id                 UUID                            NOT NULL
);

-- ============================================================
--   Table : CAMION                                            
-- ============================================================
CREATE TABLE CAMION (
  PRIMARY KEY (camion_id),
  camion_id                 UUID DEFAULT gen_random_uuid()  NOT NULL,
  immatriculation           VARCHAR(15)                             ,
  type                      VARCHAR(50)                             ,
  date_mise_service         DATE                                    ,
  date_achat                DATE                                    ,
  kilometrage               INTEGER                                 ,
  capacite                  FLOAT                                   ,
  etat                      VARCHAR(20)
);

-- Ajouter les contraintes de validité pour `kilometrage` et `capacite`
ALTER TABLE CAMION ADD CONSTRAINT check_capacite CHECK (capacite > 0);
ALTER TABLE CAMION ADD CONSTRAINT check_kilometrage CHECK (kilometrage >= 0);

-- ============================================================
--   Table : CHAUFFEUR                                            
-- ============================================================
CREATE TABLE CHAUFFEUR (
  PRIMARY KEY (chauffeur_id),
  chauffeur_id              UUID DEFAULT gen_random_uuid()  NOT NULL,
  nom_chauffeur             VARCHAR(50)                             ,
  prenom_chauffeur          VARCHAR(50)                             ,
  adresse_avec_ville        VARCHAR(100)                            ,
  date_embauche             DATE                                    ,
  type_permis               VARCHAR(10)                             
);

-- ============================================================
--   Table : CONDUIRE                                            
-- ============================================================
CREATE TABLE CONDUIRE (
  PRIMARY KEY (chauffeur_id, camion_id),
  chauffeur_id              UUID                            NOT NULL,
  camion_id                 UUID                            NOT NULL
);

-- ============================================================
--   Table : CONTENIR                                            
-- ============================================================
CREATE TABLE CONTENIR (
  PRIMARY KEY (livraison_id, produit_id),
  livraison_id              UUID                            NOT NULL,
  produit_id              UUID                            NOT NULL,
  quantite                  INTEGER
);

-- ============================================================
--   Table : DEPOT                                            
-- ============================================================
CREATE TABLE DEPOT (
  PRIMARY KEY (depot_id),
  depot_id          UUID DEFAULT gen_random_uuid()  NOT NULL,
  intitule_depot            VARCHAR(50)                             ,
  ville                     VARCHAR(50)
);

-- Ajouter une contrainte d'unicité sur le nom du dépôt dans la même ville
ALTER TABLE DEPOT ADD CONSTRAINT unique_depot_nom UNIQUE (intitule_depot, ville);

-- ============================================================
--   Table : LIVRAISON                                            
-- ============================================================
CREATE TABLE LIVRAISON (
  PRIMARY KEY (livraison_id),
  livraison_id              UUID DEFAULT gen_random_uuid()  NOT NULL,
  statut_livraison          VARCHAR(20)                             ,
  camion_id                 UUID NOT NULL,
  chauffeur_id              UUID                            NOT NULL,
  depot_depart_id       UUID                            NOT NULL,
  date_prevue_depart        TIMESTAMP                               ,
  date_effective_depart     TIMESTAMP                               ,
  depot_arrivee_id      UUID                            NOT NULL,
  date_prevue_arrivee       TIMESTAMP                               ,
  date_effective_arrivee    TIMESTAMP
);


-- ============================================================
--   Table : MARQUE                                            
-- ============================================================
CREATE TABLE MARQUE (
  PRIMARY KEY (marque_id),
  marque_id                 UUID DEFAULT gen_random_uuid()  NOT NULL,
  nom_marque                VARCHAR(50)
);

-- ============================================================
--   Table : MODELE                                            
-- ============================================================
CREATE TABLE MODELE (
  PRIMARY KEY (modele_id),
  modele_id                 UUID DEFAULT gen_random_uuid()  NOT NULL,
  nom_modele                VARCHAR(50)                             ,
  marque_id                 UUID NOT NULL
);

-- Ajouter une contrainte d'unicité sur `nom_modele`
ALTER TABLE MODELE ADD CONSTRAINT unique_nom_modele UNIQUE (nom_modele);

-- ============================================================
--   Table : PRODUIT                                            
-- ============================================================
CREATE TABLE PRODUIT (
  PRIMARY KEY (produit_id),
  produit_id              UUID DEFAULT gen_random_uuid()  NOT NULL,
  nom_produit               VARCHAR(50)                             ,
  poids                     FLOAT
);

-- ============================================================
--   Table : distance                                         
-- ============================================================
CREATE TABLE distance(
  PRIMARY KEY (depot1_id, depot2_id),
  depot1_id             UUID NOT NULL,
  depot2_id           UUID NOT NULL,
  distance                  FLOAT
);

-- Ajouter des contraintes de cascade pour les suppressions sur les dépôts
ALTER TABLE DISTANCE ADD CONSTRAINT fk_depot1 FOREIGN KEY (depot1_id) REFERENCES DEPOT (depot_id) ON DELETE CASCADE;
ALTER TABLE DISTANCE ADD CONSTRAINT fk_depot2 FOREIGN KEY (depot2_id) REFERENCES DEPOT (depot_id) ON DELETE CASCADE;

-- ============================================================
--   Table : absence                                         
-- ============================================================
CREATE TABLE absence(
  PRIMARY KEY (absence_id),     
  absence_id            UUID DEFAULT gen_random_uuid()  NOT NULL,
  date_debut                DATE,
  date_fin                  DATE,
  chauffeur_id              UUID                            NOT NULL
);

-- Ajouter des contraintes sur les colonnes `date_debut` et `date_fin`
ALTER TABLE ABSENCE ALTER COLUMN date_debut SET NOT NULL;
ALTER TABLE ABSENCE ALTER COLUMN date_fin SET NOT NULL;

-- ============================================================
--   Table : infraction                                         
-- ============================================================
CREATE TABLE infraction(
  PRIMARY KEY (infraction_id),
  infraction_id         UUID DEFAULT gen_random_uuid()  NOT NULL,
  date_infraction           DATE,
  type_infraction           VARCHAR(100),
  chauffeur_id              UUID                            NOT NULL
);

-- Ajouter des contraintes sur les colonnes `date_infraction` et `type_infraction`
ALTER TABLE INFRACTION ALTER COLUMN date_infraction SET NOT NULL;
ALTER TABLE INFRACTION ALTER COLUMN type_infraction SET NOT NULL;

-- =======================================
--   Clés étrangères et actions en cascade
-- =======================================

ALTER TABLE ABSENCE ADD FOREIGN KEY (chauffeur_id) REFERENCES CHAUFFEUR (chauffeur_id) ON DELETE CASCADE;

ALTER TABLE INFRACTION ADD FOREIGN KEY (chauffeur_id) REFERENCES CHAUFFEUR (chauffeur_id) ON DELETE CASCADE;

ALTER TABLE APPARTENIR ADD FOREIGN KEY (camion_id) REFERENCES CAMION (camion_id) ON DELETE CASCADE;
ALTER TABLE APPARTENIR ADD FOREIGN KEY (modele_id) REFERENCES MODELE (modele_id) ON DELETE CASCADE;

ALTER TABLE CONDUIRE ADD FOREIGN KEY (camion_id) REFERENCES CAMION (camion_id) ON DELETE CASCADE;
ALTER TABLE CONDUIRE ADD FOREIGN KEY (chauffeur_id) REFERENCES CHAUFFEUR (chauffeur_id) ON DELETE CASCADE;

ALTER TABLE CONTENIR ADD FOREIGN KEY (produit_id) REFERENCES PRODUIT (produit_id);
ALTER TABLE CONTENIR ADD FOREIGN KEY (livraison_id) REFERENCES LIVRAISON (livraison_id);

ALTER TABLE LIVRAISON ADD FOREIGN KEY (depot_depart_id) REFERENCES DEPOT (depot_id);
ALTER TABLE LIVRAISON ADD FOREIGN KEY (depot_arrivee_id) REFERENCES DEPOT (depot_id);
ALTER TABLE LIVRAISON ADD FOREIGN KEY (chauffeur_id) REFERENCES CHAUFFEUR (chauffeur_id);
ALTER TABLE LIVRAISON ADD FOREIGN KEY (camion_id) REFERENCES CAMION (camion_id);

ALTER TABLE DISTANCE ADD FOREIGN KEY (depot1_id) REFERENCES DEPOT (depot_id);
ALTER TABLE DISTANCE ADD FOREIGN KEY (depot2_id) REFERENCES DEPOT (depot_id);

ALTER TABLE MODELE ADD FOREIGN KEY (marque_id) REFERENCES MARQUE (marque_id) ON DELETE CASCADE;



CREATE OR REPLACE FUNCTION check_livraison_dates()
RETURNS TRIGGER AS $$
BEGIN
    IF NEW.date_prevue_depart >= NEW.date_prevue_arrivee THEN
        RAISE EXCEPTION 'La date prévue de départ doit être antérieure à la date prévue d''arrivée.';
    END IF;

    IF NEW.date_effective_depart IS NOT NULL AND NEW.date_effective_arrivee IS NOT NULL AND
       NEW.date_effective_depart >= NEW.date_effective_arrivee THEN
        RAISE EXCEPTION 'La date effective de départ doit être antérieure à la date effective d''arrivée.';
    END IF;

    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

CREATE TRIGGER trigger_check_livraison_dates
BEFORE INSERT OR UPDATE ON livraison
FOR EACH ROW
EXECUTE FUNCTION check_livraison_dates();

CREATE OR REPLACE FUNCTION check_absence_dates()
RETURNS TRIGGER AS $$
BEGIN
    IF NEW.date_debut > NEW.date_fin THEN
        RAISE EXCEPTION 'La date de début doit être antérieure ou égale à la date de fin.';
    END IF;
    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

CREATE TRIGGER trigger_check_absence_dates
BEFORE INSERT OR UPDATE ON absence
FOR EACH ROW
EXECUTE FUNCTION check_absence_dates();


CREATE OR REPLACE FUNCTION check_absence_date_embauche()
RETURNS TRIGGER AS $$
DECLARE
    date_embauche DATE;
BEGIN
    SELECT c.date_embauche INTO date_embauche
    FROM chauffeur c
    WHERE c.chauffeur_id = NEW.chauffeur_id;

    IF NEW.date_debut < date_embauche THEN
        RAISE EXCEPTION 'La date de début de l''absence doit être postérieure à la date d''embauche du chauffeur.';
    END IF;

    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

CREATE TRIGGER trigger_check_absence_date_embauche
BEFORE INSERT OR UPDATE ON absence
FOR EACH ROW
EXECUTE FUNCTION check_absence_date_embauche();


CREATE OR REPLACE FUNCTION update_kilometrage()
RETURNS TRIGGER AS $$
DECLARE
    distance_km INTEGER;
BEGIN
    IF NEW.statut_livraison = 'Terminée' THEN
        SELECT distance
        INTO distance_km
        FROM distance
        WHERE (distance.depot1_id = NEW.depot_depart_id
               AND distance.depot2_id = NEW.depot_arrivee_id)
           OR (distance.depot2_id = NEW.depot_depart_id
               AND distance.depot1_id = NEW.depot_arrivee_id);

        IF distance_km IS NOT NULL THEN
            UPDATE camion
            SET kilometrage = kilometrage + distance_km
            WHERE camion.camion_id = NEW.camion_id;
        END IF;
    END IF;
    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

CREATE TRIGGER trigger_update_kilometrage
BEFORE UPDATE ON livraison
FOR EACH ROW
WHEN (OLD.statut_livraison = 'En cours' AND NEW.statut_livraison = 'Terminée')
EXECUTE FUNCTION update_kilometrage();

CREATE OR REPLACE FUNCTION check_camion_disponible()
RETURNS TRIGGER AS $$
BEGIN
    IF EXISTS (
        SELECT 1
        FROM livraison l
        WHERE l.camion_id = NEW.camion_id
        AND l.livraison_id <> NEW.livraison_id  
        AND (
            NEW.date_prevue_depart < l.date_prevue_arrivee
            AND NEW.date_prevue_arrivee > l.date_prevue_depart
        )
    ) THEN
        RAISE EXCEPTION 'Le camion est déjà assigné à une autre livraison durant cette période.';
    END IF;
    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

CREATE TRIGGER trigger_check_camion_disponible
BEFORE INSERT OR UPDATE ON livraison
FOR EACH ROW
EXECUTE FUNCTION check_camion_disponible();

CREATE OR REPLACE FUNCTION check_chauffeur_disponible()
RETURNS TRIGGER AS $$
BEGIN
    IF EXISTS (
        SELECT 1
        FROM livraison l
        WHERE l.chauffeur_id = NEW.chauffeur_id
        AND l.livraison_id <> NEW.livraison_id  -- Ne pas considérer la même livraison en cas de mise à jour
        AND (
            NEW.date_prevue_depart < l.date_prevue_arrivee
            AND NEW.date_prevue_arrivee > l.date_prevue_depart
        )
    ) THEN
        RAISE EXCEPTION 'Le chauffeur est déjà assigné à une autre livraison durant cette période.';
    END IF;
    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

CREATE TRIGGER trigger_check_chauffeur_disponible
BEFORE INSERT OR UPDATE ON livraison
FOR EACH ROW
EXECUTE FUNCTION check_chauffeur_disponible();

CREATE OR REPLACE FUNCTION check_contenir_poids()
RETURNS TRIGGER AS $$
DECLARE
    poids_total FLOAT;
    capacite_camion FLOAT;
BEGIN
    -- Calculer le poids total actuel des produits dans la livraison (y compris le nouveau produit ou la mise à jour)
    SELECT SUM(p.poids * c.quantite)
    INTO poids_total
    FROM contenir c
    JOIN produit p ON c.produit_id = p.produit_id
    WHERE c.livraison_id = NEW.livraison_id;

    -- Récupérer la capacité du camion assigné à la livraison
    SELECT capacite
    INTO capacite_camion
    FROM camion
    WHERE camion_id = (
        SELECT camion_id
        FROM livraison
        WHERE livraison_id = NEW.livraison_id
    );

    -- Vérifier si le poids total dépasse la capacité du camion
    IF poids_total > capacite_camion THEN
        RAISE EXCEPTION 'Le poids total de la livraison (%.2f) dépasse la capacité du camion (%.2f).',
            poids_total, capacite_camion;
    END IF;

    RETURN NEW;
END;
$$ LANGUAGE plpgsql;


CREATE TRIGGER trigger_check_contenir_poids
AFTER INSERT OR UPDATE OR DELETE ON contenir
FOR EACH ROW
EXECUTE FUNCTION check_contenir_poids();


