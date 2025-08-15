-- Moyenne des poids transportés par camion par livraison
SELECT  poids_par_livraison_par_camion.camion_id, 
        AVG(total_poids) AS moyenne_poids_par_livraison
FROM (
    SELECT  c.livraison_id,
            l.camion_id,
            SUM(c.quantite * p.poids) AS total_poids
    FROM CONTENIR c
    JOIN PRODUIT p ON c.produit_id = p.produit_id
    JOIN LIVRAISON l ON c.livraison_id = l.livraison_id
    GROUP BY c.livraison_id, l.camion_id
) AS poids_par_livraison_par_camion
GROUP BY poids_par_livraison_par_camion.camion_id;


-- Moyenne des distances parcourues par les chauffeurs pour effectuer une livraison
SELECT  l.chauffeur_id,
        AVG(d.distance) AS moyenne_distance_par_livraison
FROM LIVRAISON l
JOIN DISTANCE d ON (l.depot_depart_id = d.depot1_id AND l.depot_arrivee_id = d.depot2_id)
                OR (l.depot_depart_id = d.depot2_id AND l.depot_arrivee_id = d.depot1_id)
GROUP BY l.chauffeur_id;


-- Classement des dépôts les plus proches d’une destination
SELECT 
source.intitule_depot AS depot_source_intitule,
source.ville AS depot_source_ville,
proche.intitule_depot AS depot_proche_intitule,
proche.ville AS depot_proche_ville,
distances.distance
FROM (
SELECT d.depot1_id AS depot_source,
        d.depot2_id AS depot_proche,
        d.distance
FROM DISTANCE d
WHERE d.depot1_id = 'DEST_DEPOT_ID'
UNION ALL
SELECT d.depot2_id AS depot_source,
        d.depot1_id AS depot_proche,
        d.distance
FROM DISTANCE d
WHERE d.depot2_id = 'DEST_DEPOT_ID'
) AS distances
JOIN DEPOT source ON distances.depot_source = source.depot_id
JOIN DEPOT proche ON distances.depot_proche = proche.depot_id
ORDER BY distances.distance ASC;


-- Classement des camions vides les plus proches d’un dépôt

WITH camions_vides_localisation AS (
    SELECT DISTINCT ON (cv.camion_id)
        cv.camion_id,
        l.depot_arrivee_id
    FROM (
        SELECT c.camion_id
        FROM CAMION c
        WHERE c.camion_id NOT IN (
            SELECT c.camion_id
            FROM CAMION c
            JOIN LIVRAISON l ON c.camion_id = l.camion_id
            WHERE l.statut_livraison = 'En cours'
        )
    ) cv
    JOIN LIVRAISON l ON cv.camion_id = l.camion_id
    WHERE l.statut_livraison = 'Terminée'
    ORDER BY cv.camion_id, l.date_effective_arrivee DESC
),
distances AS (
    SELECT 
        d.depot1_id AS depot_source_id,
        d.depot2_id AS depot_target_id,
        cl.camion_id AS camion_vide_id,
        d.distance
    FROM camions_vides_localisation cl
    JOIN DISTANCE d ON cl.depot_arrivee_id = d.depot2_id
    WHERE d.depot1_id = 'DEST_DEPOT_ID'
    UNION ALL
    SELECT 
        d.depot2_id AS depot_source_id,
        d.depot1_id AS depot_target_id,
        cl.camion_id AS camion_vide_id,
        d.distance
    FROM camions_vides_localisation cl
    JOIN DISTANCE d ON cl.depot_arrivee_id = d.depot1_id
    WHERE d.depot2_id = 'DEST_DEPOT_ID'
)
SELECT 
    ds.intitule_depot AS intitule_depot_source,
    c.immatriculation AS immatriculation_camion,
    d.distance
FROM distances d
JOIN DEPOT ds ON ds.depot_id = d.depot_source_id
JOIN CAMION c ON c.camion_id = d.camion_vide_id
ORDER BY d.distance ASC;


-- Classement des chauffeurs
SELECT  ch.nom_chauffeur, 
        ch.prenom_chauffeur,
        pénalité_absence.total_jours_absence * 3.2 + pénalité_infraction.nombre_infractions * 5 AS note
FROM CHAUFFEUR ch
JOIN (
    SELECT  a.chauffeur_id,
            SUM(EXTRACT(EPOCH FROM (date_fin::timestamp - date_debut::timestamp)) / 86400) AS total_jours_absence
    FROM ABSENCE a
    GROUP BY a.chauffeur_id
) pénalité_absence 
ON pénalité_absence.chauffeur_id = ch.chauffeur_id
JOIN (
    SELECT  i.chauffeur_id,
            COUNT(i.infraction_id) AS nombre_infractions
    FROM INFRACTION i
    GROUP BY i.chauffeur_id
) pénalité_infraction
ON pénalité_infraction.chauffeur_id = pénalité_absence.chauffeur_id
ORDER BY note ASC;

-- ============================================================
--   REQUEST: SELECT (CONSULTATION)            
-- ============================================================

--CHAUFFEURS
SELECT nom_chauffeur, prenom_chauffeur,addresse_avec_ville, date_embauche, type_permis FROM 
CHAUFFEUR;

--CAMIONS
SELECT camion.immatriculation, camion.type, camion.date_mise_en_service, camion.date_achat, 
camion.kilometrage, camion.capacite, camion.etat, modele.nom_modele, marque.nom_marque 
camion.camion_id 
FROM camion JOIN appartenir ON camion.camion_id =
appartenir.camion_id JOIN modele ON appartenir.modele_id = modele.modele_id JOIN marque 
ON modele.marque_id = marque.marque_id;

--LIVRAISONS
SELECT statut_livraison, immatriculation, nom_chauffeur, depot_depart.intitule_depot AS intitule_depot_depart, date_prevue_depart, date_effective_depart, depot_arrivee.intitule_depot AS intitule_depot_arrivee, date_prevue_arrivee, date_effective_arrivee, livraison_id
FROM livraison 
JOIN camion ON livraison.camion_id = camion.camion_id
JOIN chauffeur ON livraison.chauffeur_id = chauffeur.chauffeur_id
JOIN depot AS depot_arrivee ON livraison.depot_arrivee_id = depot_arrivee.depot_id
JOIN depot AS depot_depart ON livraison.depot_depart_id = depot_depart.depot_id

        --PRODUITS DANS LES LIVRAISONS
        SELECT nom_produit, poids, quantite FROM livraison JOIN contenir 
        ON livraison.livraison_id = contenir.livraison_id 
        JOIN produit ON contenir.produit_id = produit.produit_id WHERE livraison.livraison_id = 'LIVRAISON_ID'

--DEPOTS
SELECT intitule_depot, ville, depot_id FROM depot;

        --DISTANCES
        SELECT 
        d2.intitule_depot AS autre_depot_nom,
        d2.ville AS autre_depot_ville,
        distance.distance AS distance_km
        FROM 
        distance
        JOIN 
        depot d2 ON (distance.depot2_id = d2.depot_id OR distance.depot1_id = d2.depot_id)
        WHERE 
        (distance.depot1_id = "DEPOT_ID" OR distance.depot2_id = "DEPOT_ID")
        AND d2.depot_id != "DEPOT_ID";

--PRODUITS
SELECT nom_produit, poids, produit_id FROM produit;

--INFRACTIONS
SELECT date_infraction, type_infraction, nom_chauffeur, infraction_id 
      FROM infraction 
      JOIN chauffeur ON infraction.chauffeur_id = chauffeur.chauffeur_id;

--ABSENCES
SELECT nom_chauffeur, date_debut, date_fin, absence_id
      FROM absence 
      JOIN chauffeur ON absence.chauffeur_id = chauffeur.chauffeur_id;




