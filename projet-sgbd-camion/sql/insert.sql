-- Insérer des marques
INSERT INTO MARQUE (nom_marque) VALUES 
('Mercedes-Benz'),
('Volvo'),
('Scania'),
('MAN'),
('Iveco');

-- Insérer des modèles
INSERT INTO MODELE (nom_modele, marque_id) VALUES 
('Actros', (SELECT marque_id FROM MARQUE WHERE nom_marque = 'Mercedes-Benz')),
('FH16', (SELECT marque_id FROM MARQUE WHERE nom_marque = 'Volvo')),
('R Series', (SELECT marque_id FROM MARQUE WHERE nom_marque = 'Scania')),
('TGX', (SELECT marque_id FROM MARQUE WHERE nom_marque = 'MAN')),
('Stralis', (SELECT marque_id FROM MARQUE WHERE nom_marque = 'Iveco'));

-- Insérer des dépôts
INSERT INTO DEPOT (intitule_depot, ville) VALUES 
('Depot Nord', 'Paris'),
('Depot Sud', 'Marseille'),
('Depot Est', 'Strasbourg'),
('Depot Ouest', 'Bordeaux'),
('Depot Central', 'Lyon');

-- Insérer des camions
INSERT INTO CAMION (immatriculation, type, date_mise_service, date_achat, kilometrage, capacite, etat) VALUES 
('AB-123-CD', 'Remorque', '2020-03-15', '2020-01-10', 120000, 15.5, 'En service'),
('EF-456-GH', 'Semi-remorque', '2019-06-20', '2019-05-05', 200000, 18.0, 'En service'),
('IJ-789-KL', 'Fourgon', '2021-10-01', '2021-08-20', 50000, 12.3, 'En service'),
('MN-012-OP', 'Citerne', '2018-02-10', '2018-01-15', 300000, 20.0, 'En service'),
('QR-345-ST', 'Benne', '2022-07-12', '2022-06-30', 30000, 10.0, 'En service');

-- Insérer des chauffeurs
INSERT INTO CHAUFFEUR (nom_chauffeur, prenom_chauffeur, adresse_avec_ville, date_embauche, type_permis) VALUES 
('Dupont', 'Jean', '12 Rue de Paris, Paris', '2015-04-10', 'C'),
('Martin', 'Pierre', '34 Avenue de la République, Marseille', '2018-09-05', 'CE'),
('Bernard', 'Paul', '56 Boulevard de Lyon, Lyon', '2017-11-20', 'C1E'),
('Durand', 'Jacques', '78 Rue de Strasbourg, Strasbourg', '2020-01-15', 'D'),
('Petit', 'Marie', '90 Rue de Bordeaux, Bordeaux', '2021-03-30', 'B');



-- Insertion des absence
INSERT INTO absence (chauffeur_id, date_debut, date_fin) 
VALUES 
((SELECT chauffeur_id FROM CHAUFFEUR LIMIT 1), '2024-06-01', '2024-06-10'),
((SELECT chauffeur_id FROM CHAUFFEUR LIMIT 1 OFFSET 1), '2024-07-15', '2024-07-20'),
((SELECT chauffeur_id FROM CHAUFFEUR LIMIT 1 OFFSET 2), '2024-08-05', '2024-08-12'),
((SELECT chauffeur_id FROM CHAUFFEUR LIMIT 1 OFFSET 3), '2024-09-10', '2024-09-15'),
((SELECT chauffeur_id FROM CHAUFFEUR LIMIT 1 OFFSET 4), '2024-10-01', '2024-10-05');

-- Insertion des infraction
INSERT INTO infraction (chauffeur_id, date_infraction, type_infraction) 
VALUES 
((SELECT chauffeur_id FROM CHAUFFEUR LIMIT 1), '2024-05-12', 'Excès de vitesse'),
((SELECT chauffeur_id FROM CHAUFFEUR LIMIT 1 OFFSET 1), '2024-06-25', 'Non-port de la ceinture'),
((SELECT chauffeur_id FROM CHAUFFEUR LIMIT 1 OFFSET 2), '2024-07-30', 'Téléphone au volant'),
((SELECT chauffeur_id FROM CHAUFFEUR LIMIT 1 OFFSET 3), '2024-08-18', 'Dépassement interdit'),
((SELECT chauffeur_id FROM CHAUFFEUR LIMIT 1 OFFSET 4), '2024-09-02', 'Stationnement interdit');


-- Insertion des distances entre dépôts
INSERT INTO distance (depot1_id, depot2_id, distance) 
VALUES 
((SELECT depot_id FROM DEPOT WHERE ville = 'Paris'), (SELECT depot_id FROM DEPOT WHERE ville = 'Marseille'), 775),
((SELECT depot_id FROM DEPOT WHERE ville = 'Paris'), (SELECT depot_id FROM DEPOT WHERE ville = 'Lyon'), 465),
((SELECT depot_id FROM DEPOT WHERE ville = 'Paris'), (SELECT depot_id FROM DEPOT WHERE ville = 'Strasbourg'), 488),
((SELECT depot_id FROM DEPOT WHERE ville = 'Marseille'), (SELECT depot_id FROM DEPOT WHERE ville = 'Lyon'), 315),
((SELECT depot_id FROM DEPOT WHERE ville = 'Marseille'), (SELECT depot_id FROM DEPOT WHERE ville = 'Strasbourg'), 813),
((SELECT depot_id FROM DEPOT WHERE ville = 'Lyon'), (SELECT depot_id FROM DEPOT WHERE ville = 'Bordeaux'), 554),
((SELECT depot_id FROM DEPOT WHERE ville = 'Bordeaux'), (SELECT depot_id FROM DEPOT WHERE ville = 'Paris'), 584),
((SELECT depot_id FROM DEPOT WHERE ville = 'Strasbourg'), (SELECT depot_id FROM DEPOT WHERE ville = 'Bordeaux'), 900);

-- Insertion de 100 produits avec marques et intitulés détaillés dans la tdepot_id
DO $$
DECLARE
  produits_noms TEXT[] := ARRAY[
    'Eau minérale Evian',
    'Eau gazeuse Perrier',
    'Coca-Cola classique',
    'Coca-Cola zéro',
    'Fanta Orange',
    'Sprite',
    'Jus d''orange Tropicana',
    'Jus de pomme Pago',
    'Bière blonde Heineken',
    'Bière brune Guinness',
    'Chocolat noir Lindt',
    'Chocolat au lait Milka',
    'Biscuit sablé Petit Écolier',
    'Biscuit chocolat Oreo',
    'Café moulu Carte Noire',
    'Café instantané Nescafé',
    'Thé vert Lipton',
    'Lait entier Lactel',
    'Yaourt nature Danone',
    'Céréales complètes Special K',
    'Céréales muesli Jordan''s',
    'Chips salées Lay''s',
    'Chips saveur barbecue Pringles'
  ];

  produits_poids FLOAT[] := ARRAY[
    1.0, 1.0, 1.5, 1.5, 1.5, 1.5,
    1.0, 1.0, 0.5, 0.5, 0.2, 0.2,
    0.1, 0.1, 0.2, 0.1, 0.1, 1.0,
    0.15, 0.5, 0.5, 0.1, 0.1
  ];

  i INTEGER;
BEGIN
  -- Insérer chaque produit
  FOR i IN 1..array_length(produits_noms, 1) LOOP
    INSERT INTO PRODUIT (nom_produit, poids) 
    VALUES (produits_noms[i], produits_poids[i]);
  END LOOP;
END $$;

-- Insérer des livraisons
INSERT INTO LIVRAISON (statut_livraison, camion_id, chauffeur_id, depot_depart_id, date_prevue_depart, date_effective_depart, depot_arrivee_id, date_prevue_arrivee, date_effective_arrivee) VALUES 
('En cours', (SELECT camion_id FROM CAMION LIMIT 1), (SELECT chauffeur_id FROM CHAUFFEUR LIMIT 1),  (SELECT depot_id FROM DEPOT WHERE ville = 'Paris'), '2024-10-10 08:00', '2024-10-10 08:30', (SELECT depot_id FROM DEPOT WHERE ville = 'Marseille'), '2024-10-10 18:00', '2024-10-10 17:45'),
('Terminée', (SELECT camion_id FROM CAMION LIMIT 1 OFFSET 1), (SELECT chauffeur_id FROM CHAUFFEUR LIMIT 1 OFFSET 1), (SELECT depot_id FROM DEPOT WHERE ville = 'Lyon'), '2024-09-20 09:00', '2024-09-20 09:10', (SELECT depot_id FROM DEPOT WHERE ville = 'Strasbourg'), '2024-09-20 15:00', '2024-09-20 14:45'),
('En attente', (SELECT camion_id FROM CAMION LIMIT 1 OFFSET 2), (SELECT chauffeur_id FROM CHAUFFEUR LIMIT 1 OFFSET 2), (SELECT depot_id FROM DEPOT WHERE ville = 'Paris'), '2024-12-05 08:00', NULL, (SELECT depot_id FROM DEPOT WHERE ville = 'Bordeaux'), '2024-12-05 18:00', NULL),
('En cours', (SELECT camion_id FROM CAMION LIMIT 1 OFFSET 3), (SELECT chauffeur_id FROM CHAUFFEUR LIMIT 1 OFFSET 3), (SELECT depot_id FROM DEPOT WHERE ville = 'Bordeaux'), '2024-12-06 07:00', '2024-12-06 07:15', (SELECT depot_id FROM DEPOT WHERE ville = 'Strasbourg'), '2024-12-06 19:00', NULL),
('Terminée', (SELECT camion_id FROM CAMION LIMIT 1 OFFSET 4), (SELECT chauffeur_id FROM CHAUFFEUR LIMIT 1 OFFSET 4), (SELECT depot_id FROM DEPOT WHERE ville = 'Marseille'), '2024-11-01 10:00', '2024-11-01 10:10', (SELECT depot_id FROM DEPOT WHERE ville = 'Paris'), '2024-11-01 15:00', '2024-11-01 14:50');


-- Insérer des relations "APPARTENIR" (modèle à camion)
INSERT INTO APPARTENIR (modele_id, camion_id) 
SELECT modele_id, camion_id FROM MODELE, CAMION LIMIT 5;

-- Insérer des relations "CONDUIRE" (chauffeur à camion)
INSERT INTO CONDUIRE (chauffeur_id, camion_id) 
SELECT chauffeur_id, camion_id FROM CHAUFFEUR, CAMION LIMIT 5;

-- Insérer des relations "CONTENIR" (livraison à produit)
INSERT INTO CONTENIR (livraison_id, produit_id, quantite) 
SELECT livraison_id, produit_id, FLOOR(RANDOM() * 5 + 1) 
FROM LIVRAISON, PRODUIT 
WHERE RANDOM() < 0.3 -- Réduit la probabilité de sélectionner une combinaison
LIMIT 3; -- Réduit le nombre total de lignes insérées
