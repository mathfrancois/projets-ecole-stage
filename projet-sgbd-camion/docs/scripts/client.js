// Clic sur une section ==> affiche la section
function showSection(sectionId) {
    const sections = document.querySelectorAll('.section');
    sections.forEach(section => section.classList.add('hidden'));
    const sectionToShow = document.getElementById(sectionId);
    if (sectionToShow) {
        sectionToShow.classList.remove('hidden');
    }
}


// Gestion de la case "Tout sélectionner" pour chaque table
function manageSelectAll(table) {
    
    document.getElementById(`selectAll${table}`).addEventListener('change', function() {
        
        const section = document.getElementById(`section${table}`); 
        section.querySelectorAll('input[type="checkbox"]:not(#selectAll)').forEach(checkbox => {
            checkbox.checked = this.checked;
        });
    });
}

// Activer les selectAll Pour chaque table
manageSelectAll('chauffeur');
manageSelectAll('camion');
manageSelectAll('livraison');
manageSelectAll('depot');
manageSelectAll('produit');
manageSelectAll('infraction');
manageSelectAll('absence');



// Recupère quels attributs sont cochés pour la table table
function getAttributesTicked(table) {
    const form = document.getElementById(`${table}-attributs-form`);
    if (!form) return [];
    return Array.from(form.querySelectorAll('input[type="checkbox"]:checked:not(#selectAll)'))
        .map(checkbox => checkbox.name)  
        .filter(attribute => attribute.trim() !== '');  
}

function generateTableHTML(attributes, entreeList) {
    const table = entreeList.table;  
    attributes = attributes.filter(attribute => attribute && attribute.trim() !== '');
    let contentHTML = `<div class='ligne nomsattributs'>`;  
    attributes.forEach(attribute => {
        if (attribute === 'camion_id') attribute = 'immatriculation';
        if (attribute === 'chauffeur_id') attribute = 'nom_chauffeur';
        if (attribute === 'depot_depart_id') attribute = 'intitule_depot_depart';
        if (attribute === 'depot_arrivee_id') attribute = 'intitule_depot_arrivee';
        contentHTML += `<p class='ligne entete'>${attribute}</p>`;
    });
     contentHTML += `<p class='ligne entete'>Modifications</p>`
    contentHTML += `</div>`; 
    console.log(JSON.stringify(entreeList, null, 2));

    console.log("MODIFHTML"); 
    console.log(entreeList); 
    const dataEntreeList = entreeList.data;
    contentHTML += `<div class="table-data">`;
    dataEntreeList.forEach(entree => {
        contentHTML += `<div class='ligne entree'>`;  
        attributes.forEach(attribute => {
            if (attribute === 'camion_id') attribute = 'immatriculation';
            if (attribute === 'chauffeur_id') attribute = 'nom_chauffeur';
            if (attribute === 'depot_depart_id') attribute = 'intitule_depot_depart';
            if (attribute === 'depot_arrivee_id') attribute = 'intitule_depot_arrivee';
            const value = entree[attribute.toString()] || '';
            // Si c'est le statut de livraison, ajoute le select et le bouton
            if (attribute === "statut_livraison") {
                const selectId = `statut_livraison_${entree.livraison_id}`;
                contentHTML += `
                    <p class="attribut statut-container">
                        <select class="attribut selectlivraisonstate" name='statut_livraison' id='${selectId}' data-livraison-id='${entree.livraison_id}'>
                            <option value='En attente' ${entree.statut_livraison === 'En attente' ? 'selected' : ''}>En attente</option>
                            <option value='En cours' ${entree.statut_livraison === 'En cours' ? 'selected' : ''}>En cours</option>
                            <option value='Terminée' ${entree.statut_livraison === 'Terminée' ? 'selected' : ''}>Terminée</option>
                        </select>
                    </p>`;
            } else {
                contentHTML += `<p class='attribut'>${value}</p>`;  
            }
        });
        const tableId = `${table}_id`;  
        const entreeId = entree[`${tableId}`];
        if (table){
            contentHTML += `
            <p class="attribut button-container modifbuttoncontainer">
            <button 
                class="delete-btn modifbutton" 
                data-id="${entreeId}" 
                data-table="${table}">
                Supprimer
            </button>`;
            if (table === 'livraison') {
                contentHTML += `
                <button 
                    class="show-btn modifbutton" 
                    data-id="${entreeId}" 
                    data-table="${table}">
                    Voir les produits
                </button>
                <button 
                    class="addproduct-btn modifbutton" 
                    data-id="${entreeId}" 
                    data-table="${table}">
                    Ajouter des produits à la livraison
                </button>`;
            }
            if (table === "depot"){
                contentHTML += `
                <button 
                    class="show-btn modifbutton" 
                    data-id="${entreeId}" 
                    data-table="${table}">
                    Voir les distances aux autres dépôts
                </button>`;
            }
            contentHTML += `</p>`;
        }
        
        
        
        contentHTML += `</div>`;
    });

    return contentHTML;
}

function openPopup(contentHTML) {
    const popup = document.getElementById('popup');
    const popupData = document.getElementById('popup-data');
    popupData.innerHTML = contentHTML; // Insère le contenu dynamique dans la popup
    popup.classList.remove('hidden'); // Affiche la popup
}

document.getElementById('popup-close').addEventListener('click', closePopup);

function closePopup() {
    const popup = document.getElementById('popup');
    popup.classList.add('hidden'); // Masque la popup
}

// Gestionnaire d'événements pour le bouton "show"
document.addEventListener('click', async function(event) {
    if (event.target.matches('.show-btn')) {
        const id = event.target.getAttribute('data-id');
        const table = event.target.getAttribute('data-table');
        let attributes = "nom_produit,quantite,poids"; // Attributs à récupérer pour les produits
        if (table === 'depot') {
            attributes = "autre_depot_nom,autre_depot_ville,distance_km";
        }
        
        // Appel API pour récupérer les données
        try {
            const response = await fetch(`/api/${table}/details`, {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json'
                },
                body: JSON.stringify({ id, table })
            });

            if (!response.ok) {
                throw new Error('Erreur lors de la récupération des données');
            }

            const data = await response.json();
            // Générer le HTML pour afficher les données
            let contentHTML;
            if (table === 'produit') {
                contentHTML = `<h2>Détails des produits</h2>`;
            }
            if (table === 'depot') {
                contentHTML = `<h2>Détails des distances aux autres dépôts</h2>`;
            }
            contentHTML += generateTableHTML(attributes.split(','), data);
            // Ouvrir le popup avec les données
            openPopup(contentHTML);
        } catch (error) {
            console.error('Erreur:', error);
            alert('Impossible de charger les données');
        }
    }
    if (event.target.matches('.addproduct-btn')) {
        const id = event.target.getAttribute('data-id');
        if (!id) {
            console.error("ID de livraison introuvable ou invalide.");
            return;
        }    
        let contentHTML = `
        <div class="form-container">
            <h2>Ajouter un produit à la livraison</h2>
            <label for="produit_id_sel_pop">Produit</label>
            <select id="produit_id_sel_pop">
            </select><br>
            <label for="quantite_pop">Quantité</label>
            <input type="number" id="quantite_pop" name="quantite" required>
            <button onclick="addProductToLivraison('${String(id)}')">Ajouter</button>`;
        openPopup(contentHTML);
        loadDropdowns('produit', 'produit_id_sel_pop');
    }
});

function addProductToLivraison(idLivraison){
    const product = document.getElementById("produit_id_sel_pop");
    if (!product) {
        console.error("Element produit_id introuvable pour produit");
        return;
    }
    const produit_id = product.value;
    const quantite = document.getElementById("quantite_pop").value;
    if (quantite <= 0) {
        alert("La quantité doit être supérieure à 0");
        return;
    }
    console.log(`Ajout du produit ${produit_id} à la livraison ${idLivraison} avec quantité ${quantite}`);
    try{
        fetch(`/api/livraison/addproduct`, {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json'
            },
            body: JSON.stringify({ idLivraison, produit_id, quantite })
        })
        .then(response => {
            if (!response.ok) {
                throw new Error(`Erreur réseau : ${response.status} ${response.statusText}`);
            }
            return response.json();
        })
        .then(data => {
            console.log('Produit ajouté avec succès:', data);
            alert(`Produit ajouté avec succès à la livraison #${idLivraison}`);
            closePopup();
        })
    }
    catch(error){
        console.error('Erreur lors de l\'ajout du produit:', error);
        alert(`Erreur lors de l'ajout du produit à la livraison #${idLivraison}`);
    }
}

// Gestionnaire d'événements global pour les boutons "Modifier"
document.addEventListener('change', function(event) {
    
    if (event.target.matches('.selectlivraisonstate')) {
        const livraisonId = event.target.getAttribute('data-livraison-id');
        console.log(livraisonId); 
        if (livraisonId) {
            updateLivraison(livraisonId);
        }
    }
});


document.addEventListener('click', async function(event) {
    if (event.target.matches('.delete-btn')) {
        const id = event.target.getAttribute('data-id');
        const table = event.target.getAttribute('data-table');

        if (id && table) {
            if (confirm(`Voulez-vous vraiment supprimer cet élément ?`)) {
                try {
                    const response = await fetch(`/api/delete`, {
                        method: 'DELETE',
                        headers: { 'Content-Type': 'application/json' },
                        body: JSON.stringify({ id, table})
                    });

                    if (!response.ok) throw new Error('Erreur lors de la suppression');
                    alert(`Élément supprimé avec succès`);
                    displayTable(table); // Recharge la table pour refléter les changements
                } catch (error) {
                    console.error('Erreur lors de la suppression:', error);
                    alert(`Erreur lors de la suppression de l'élément`);
                }
            }
        }
    }
});

// Fonction updateLivraison modifiée pour une meilleure gestion
function updateLivraison(livraisonId) {
    const selectElement = document.getElementById(`statut_livraison_${livraisonId}`);
    if (!selectElement) {
        console.error(`Élément select introuvable pour livraison ID ${livraisonId}`);
        return;
    }

    const statut = selectElement.value;

    console.log(`Mise à jour du statut pour livraison ID ${livraisonId} avec le statut "${statut}"`);

    fetch(`/api/livraisonUpdate`, {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify({
            livraison_id: livraisonId,
            statut_livraison: statut
        })
    })
    .then(response => {
        if (!response.ok) {
            throw new Error(`Erreur réseau : ${response.status} ${response.statusText}`);
        }
        return response.json();
    })
    .then(data => {
        console.log('Statut mis à jour avec succès:', data);
        alert(`Livraison #${livraisonId} mise à jour avec succès au statut "${statut}"`);
    })
    .catch(error => {
        console.error('Erreur lors de la mise à jour:', error);
        alert(`Erreur lors de la mise à jour de la livraison #${livraisonId}.`);
    });
}




//affiche la table table, sur les attrinbuts selectionnées dans le form html
async function displayTable(table) {
    console.log(`displayTable appelée sur ${table}`); 
    try {
        const attributes = getAttributesTicked(table);
        console.log(`   avec les attributs : ${attributes}`); 

        
        const response = await fetch(`/api/${table}Get`, {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json'
            },
            body: JSON.stringify({ attributes })
        });

    
        if (!response.ok) {
            throw new Error(`Erreur réseau : ${response.status} ${response.statusText}`);
        }

        const entreeList = await response.json();

        // Récupération du conteneur pour afficher les données
        const tableList = document.getElementById(`${table}-list`);

        // On vide la div pour préparer l'affichage
        tableList.innerHTML = '';

        tableList.innerHTML = generateTableHTML(attributes, entreeList);

    } catch (error) {
        console.error("Erreur lors de la récupération des données :", error);

        // Affichage d'un message d'erreur
        const tableList = document.getElementById(`${table}-list`);
        tableList.innerHTML = '<p>Une erreur est survenue lors du chargement des données.</p>';
    }
}


// Ajoute à une table
async function addToTable(table, attributs) {
    const bodyData = {};
    Object.keys(attributs).forEach(attribute => {
        bodyData[attribute] = attributs[attribute];
    });
    const response = await fetch(`/api/${table}Add`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(bodyData)
    });
    console.log(response);
    if (!response.ok) console.error('Erreur lors de l\'ajout de l\'élément');
    else alert('Élément ajouté avec succès');
    displayTable(table); 
}

// Fonction pour charger les options du menu déroulant pour différentes tables
function loadDropdowns(table, elementId) {
    const url = `/api/${table}/ids`;
    console.log(`Chargement des ${table}s depuis ${url}`);

    fetch(url)
        .then(response => {
            console.log(`Statut HTTP pour ${url} : ${response.status}`);
            if (!response.ok) throw new Error(`Erreur HTTP ! statut : ${response.status}`);
            return response.json();
        })
        .then(data => {
            const eltSelect = document.getElementById(elementId); // Utilise le paramètre elementId
            if (!eltSelect) {
                console.error(`Élément avec l'ID ${elementId} non trouvé dans le DOM`);
                return;
            }
            eltSelect.innerHTML = '';  
            data.forEach(elt => {
                const option = document.createElement('option');
                option.value = elt.id;
                if (table === 'produit') {
                    option.textContent = `${elt.nom} (${String(elt.poids)} kg)`;
                }
                else{
                    option.textContent = `#${elt.nom}`;
                }
                eltSelect.appendChild(option);
            });
        })
        .catch(error => console.error(`Erreur lors du chargement des ${table}s:`, error));
}

//Pour insertion dans camion
loadDropdowns('modele', 'nom_modele_sel');

// Pour insertion dans livraison
loadDropdowns('chauffeur', 'chauffeur_id_sel_1');  
loadDropdowns('camion', 'camion_id_sel_1');
loadDropdowns('depot','depot_depart_id_sel');
loadDropdowns('depot','depot_arrivee_id_sel');

// Dans infraction
loadDropdowns('chauffeur', 'chauffeur_id_sel_2');  

// Dans absences
loadDropdowns('chauffeur', 'chauffeur_id_sel_3');  


// CHAUFFEURS
async function addChauffeur() {

    const attributs = {
        nom_chauffeur: document.getElementById("nom_chauffeur").value,
        prenom_chauffeur: document.getElementById("prenom_chauffeur").value,
        adresse_avec_ville: document.getElementById("adresse_avec_ville").value,
        date_embauche: document.getElementById("date_embauche").value,
        type_permis: document.getElementById("type_permis").value,
    }; 
    console.log(Object.values(attributs)); 
    await addToTable('chauffeur', attributs); 
    console.log('Client : Chauffeur Ajouté'); 
}

// CAMIONS
async function addCamion() {
    const attributs = {
        immatriculation: document.getElementById("immatriculation").value,
        type: document.getElementById("type").value,
        date_mise_service: document.getElementById("date_mise_service").value,
        date_achat: document.getElementById("date_achat").value,
        kilometrage: document.getElementById("kilometrage").value,
        capacite: document.getElementById("capacite").value,
        etat: document.getElementById("etat").value,
        modele: document.getElementById("nom_modele_sel").value
    };
    console.log(Object.values(attributs)); 
    // Appel à la fonction d'ajout générique (ici addToTable avec 'camion' comme table)
    await addToTable('camion', attributs);
    console.log('Client : Camion Ajouté');
}

// LIVRAISONS
async function addLivraison() {
    // Créer l'objet data avec les valeurs récupérées
    const data = {
        statut_livraison: document.getElementById('statut_livraison').value,
        chauffeur_id: document.getElementById('chauffeur_id_sel_1').value,
        camion_id: document.getElementById('camion_id_sel_1').value,
        depot_depart_id: document.getElementById('depot_depart_id_sel').value,
        date_prevue_depart: document.getElementById('date_prevue_depart').value,
        date_effective_depart: document.getElementById('date_effective_depart').value,
        depot_arrivee_id: document.getElementById('depot_arrivee_id_sel').value,
        date_prevue_arrivee: document.getElementById('date_prevue_arrivee').value,
        date_effective_arrivee: document.getElementById('date_effective_arrivee').value
    };
    console.log(Object.values(data)); 
    await addToTable('livraison', data);
    console.log('Client : Livraison Ajouté');
}

// DEPOTS 
async function addDepot() {
    const data = {
        intitule_depot: document.getElementById('intitule_depot').value,
        ville: document.getElementById('ville').value,
    };
    console.log(Object.values(data)); 
    await addToTable('depot', data);
    console.log('Client : Depots Ajouté');
}

// PRODUITS
async function addProduit() {
    const data = {
        nom_produit: document.getElementById('nom_produit').value,
        poids: document.getElementById('poids').value,
    };
    console.log(Object.values(data)); 
    await addToTable('produit', data);
    console.log('Client : produit Ajouté');
}

// INFRACTIONS 
async function addInfraction() {
    const data = {
        date_infraction: document.getElementById('date_infraction').value, 
        type_infraction: document.getElementById('type_infraction').value, 
        chauffeur_id: document.getElementById('chauffeur_id_sel_2').value
    };
    
    await addToTable('infraction', data);
    console.log('Client : produit Ajouté');
}

// ABSENCES 
async function addAbsence() {
    const data = {
        date_debut: document.getElementById('date_debut').value,
        date_fin: document.getElementById('date_fin').value, 
        chauffeur_id: document.getElementById('chauffeur_id_sel_3').value
    };
    
    await addToTable('absence', data);
    console.log('Client : absence Ajoutée');
}

// MARQUES
async function addMarque() {
    const data = {
        nom_marque: document.getElementById('nom_marque').value
    };
    
    await addToTable('marque', data);
    console.log('Client : marque ajoutée');
}

// MOELES
async function addModele() {
    const data = {
        nom_modele: document.getElementById('nom_modele').value,
        marque_id: document.getElementById('marque_id_sel').value
    };
    
    await addToTable('modele', data);
    console.log('Client : modele ajoutée');
}

// APPARTENIR
async function addAppartenir() {
    const data = {
        modele_id: document.getElementById('modele_id_sel').value,
        camion_id: document.getElementById('camion_id_sel_2').value
    };
    
    await addToTable('appartenir', data);
    console.log('Client : appartenance ajoutée');
}

// CONDUIRE
async function addConduire() {
    const data = {
        chauffeur_id: document.getElementById('chauffeur_id_sel_4').value,
        camion_id: document.getElementById('camion_id_sel_2').value
    };
    
    await addToTable('conduire', data);
    console.log('Client : conducteur ajoutée');
}

// DISTANCES
async function addDistance() {
    const data = {
        depot1_id: document.getElementById('depot_id_sel_1').value,
        depot2_id: document.getElementById('depot_id_sel_2').value, 
        distance: document.getElementById('distance').value
    };
    
    await addToTable('distance', data);
    console.log('Client : distance ajoutée');
}

// CONTENIR
async function addContenir() {
    const data = {
        livraison_id: document.getElementById('livraison_id_sel').value, 
        produit_id: document.getElementById('produit_id_sel').value,
        quantite: document.getElementById('quantite').value
    };
    
    await addToTable('contenir', data);
    console.log('Client : contenir ajouté');
}

//Requetes Spéciales
const selectRequestSpecial = document.getElementById("selectreqspecial"); 
const selectParam = document.getElementById('selectparam'); 
const selectParamTitle = document.getElementById('selectparamtitle'); 
const specialReqParamContainer = document.getElementById('specialreqparamcontainer'); 
 
async function specialRequestsSelection() {
    const selectedOption = selectRequestSpecial.selectedOptions[0]; 
    console.log("Valeur sélectionnée :", selectedOption.value);
    console.log("Classe de l'option sélectionnée :", selectedOption.className);

    if (selectedOption.className === "param") {
        specialReqParamContainer.style.justifyContent="none"; 

        selectParam.style.display = "block"; 
        loadDropdowns('depot', 'selectparam'); 
        selectParamTitle.style.display = "block"; 
        if(selectedOption.value == 4){
            selectParamTitle.innerText = "Selectionnez le paramètre destination de votre requête"; 
        }else if (selectedOption.value==5) {
            selectParamTitle.innerText = "Selectionnez le paramètre dépot de votre requête"; 
        }
    } else if (selectedOption.className === "!param") {
        specialReqParamContainer.style.justifyContent="center";
        selectParam.style.display = "none"; 
        selectParamTitle.style.display = "none"; 
    }
}

function generateTableHTMLFromResponseForSpecial(dataEntreeList, selectedOption) {
    if (!dataEntreeList || dataEntreeList.length === 0) {
        return `<p>Aucun résultat à afficher.</p>`;
    }

    // Mappage des attributs pour les noms d'attributs plus compréhensibles
    const attributeMapping = {
        'camion_id': 'immatriculation',
        'chauffeur_id': 'nom_chauffeur',
        'depot_depart_id': 'intitule_depot_depart',
        'depot_arrivee_id': 'intitule_depot_arrivee'
    };

    // Récupération des attributs
    let attributes = Object.keys(dataEntreeList[0]);
    console.log(attributes); 

    // Créer la ligne d'entête
    let contentHTML = `<div class='ligne nomsattributs'>`;  
    attributes.forEach(attribute => {
        const normalizedAttribute = attributeMapping[attribute] || attribute;  // Utilisation du mappage si disponible
        contentHTML += `<p class='ligne entete'>${normalizedAttribute}</p>`;
    });
    contentHTML += `</div>`; 

    contentHTML += `<div class="table-data">`;

    // Parcours des données
    dataEntreeList.forEach(entree => {
        contentHTML += `<div class='ligne entree'>`;  
        attributes.forEach(attribute => {
            const normalizedAttribute = attributeMapping[attribute] || attribute;  // Utilisation du mappage si disponible
            let value = entree[normalizedAttribute] || '';  // Récupère la valeur, ou vide si elle est absente

            console.log(`Attribut: ${normalizedAttribute}, Valeur: ${value}`); 

            contentHTML += `<p class='attribut'>${value}</p>`;  
        });
        contentHTML += `</div>`;
    });

    contentHTML += `</div>`;
    return contentHTML;
}


// async function IdToValue(id,table){
//     const url = `api/${table}/ids`;
//     console.log(`Chargement des ${table}s depuis ${url}`);

//     try {
//         const response = await fetch(url);
//         console.log(`Statut HTTP pour ${url} : ${response.status}`);
//         if (!response.ok) throw new Error(`Erreur HTTP ! statut : ${response.status}`);
//         const data = await response.json();
//         const elt = data.find(elt => elt.id == id);
//         console.log(elt);
//         console.log(elt.nom);
//         return elt.nom; 
//     } catch (error) {
//         console.error(`Erreur lors du chargement des ${table}s:`, error);
//     }
// }

async function sendSpecialRequest() {
    const selectedOption = selectRequestSpecial.selectedOptions[0];
    const selectedOptionValue = selectedOption.value;
    const selectedOptionClass = selectedOption.className;
    const reqSpecialList = document.getElementById('reqspecial-list');

    const requestData = {
        requestType: selectedOptionValue
    };

    if (selectedOptionClass === "param") {
        const selectedParamValue = selectParam.value;
        requestData.param = selectedParamValue;
    }

    const url = `api/specialrequests`;
    console.log("Données envoyées :", requestData);

    try {
        const response = await fetch(url, {
            method: "POST", 
            headers: {
                "Content-Type": "application/json" 
            },
            body: JSON.stringify(requestData) 
        });

        console.log(`Statut HTTP pour ${url} : ${response.status}`);

        if (!response.ok) {
            throw new Error(`Erreur HTTP ! statut : ${response.status}`);
        }

        const data = await response.json();
        console.log("Réponse du serveur :", data);
        
        reqSpecialList.innerHTML = ''; 
        reqSpecialList.innerHTML = generateTableHTMLFromResponseForSpecial(data.data, selectedOptionValue); 


    } catch (error) {
        console.error("Erreur lors de l'envoi de la requête spéciale :", error);
    }
}


selectRequestSpecial.addEventListener('change', specialRequestsSelection); 
