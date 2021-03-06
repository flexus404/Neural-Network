//--------------------------------------
#include "neural.hpp"
#include "utilities.hpp"

using namespace ia;
//-----------TrainData------------------
//--------------------------------------
/**
 *  Constructeur de la classe d'entrainement
 */
::ReadTrainData::ReadTrainData(const Text parFile)
{
	trainingDataFile_.open(parFile.c_str());						//ouverture du fichier d'entrainement
	calcNumberTrain();
}

//--------------------------------------
/**
 *  Recupere la topologie du fichier de donnee
 *  @param parTopologie tableau qui contient la topologie
 */
void ::ReadTrainData::getTopologie(std::vector<unsigned> &parTopologie)
{
	Text ligne;
	Text label;
	
	getline(trainingDataFile_, ligne);								//lecture de la premiere ligne du fichier
	std::stringstream ss(ligne);									//on stocke la ligne de le stringstream
	ss >> label;													//on copie la 1ere chaine de char. dans label
	if (this->isEOF() || label.compare("topologie:") != 0)			//si cette chaine n'est pas topologie ou vide
		abort();													//on stop
	
	while (!ss.eof()) {												//tant que l'on est pas à la fin de la ligne
		unsigned n;
		ss >> n;													//on stocke la prochaine chaine de char.
		parTopologie.push_back(n);									//on l'ajoute au vector de topologie
	}
}

//--------------------------------------
/**
 *  lis les inputs dans le fichier de donnee
 *  @param parInputVal tableau de donnee d'entree
 *  @return le nombre d'entree
 */
unsigned ::ReadTrainData::getNextInputs(T_val &parInputVal)
{
	
	parInputVal.clear();											//on vide la liste des inputs
	
	Text ligne;
	Text label;
	
	getline(trainingDataFile_, ligne);
	std::stringstream ss(ligne);
	ss >> label;
	
	if (label.compare("in:") == 0)									//si la 1ere chaine de char. est in:
	{
		double val;
		while (ss >> val) {											//tant qu'il y a des valeurs dans ss
			parInputVal.push_back(val);								//on les ajoute dans le vector d'entree
		}
	}
	return static_cast<unsigned>(parInputVal.size());				//on retourne le nb d'entree
}

//--------------------------------------
/**
 *  lis les targets de sortie
 *  @param parTargetOutputVals tableau de targets de sortie
 *  @return le nombre de target
 */
unsigned ReadTrainData::getTargetOutputs(T_val &parTargetOutputVals)	//même principe que getNextInput
{
	parTargetOutputVals.clear();
	
	Text ligne;
	Text label;
	
	getline(trainingDataFile_, ligne);
	std::stringstream ss(ligne);
	ss >> label;
	
	if (label.compare("out:") == 0)
	{
		double val;
		while (ss >> val) {
			parTargetOutputVals.push_back(val);
		}
	}
	return static_cast<unsigned>(parTargetOutputVals.size());
}

//--------------------------------------
void ReadTrainData::calcNumberTrain()
{
	Text ligne;
	unsigned i = 0;
	for (i = 0; std::getline(trainingDataFile_, ligne); i++) {
		;
	}
	trainingDataFile_.clear();
	trainingDataFile_.seekg(0, std::ios::beg);
	nbLigne_ = static_cast<unsigned>(i/2);
}

//--------------------------------------
unsigned ReadTrainData::getNumberTrain() const
{
	return nbLigne_;
}










//-----------Connection-----------------
//--------------------------------------
/**
 *  constructeur de connection
 */
::Connection::Connection()
{
	poids_ = poidsRandom();											//constructeur de connection insigne
	//std::cout << poids_ << " : " << std::endl;					//une valeur random
}

//--------------------------------------
::Connection::Connection(double par)
{
	poids_ = par;
}

//--------------------------------------
/**
 *  creer un poids random
 *  @return une valeur aléatoire entre 0 et 1
 */
double ::Connection::poidsRandom(void)
{
	return rand()/(static_cast<double>(RAND_MAX) + 1.0);			//renvoie une valeur random entre 0.0 et 1.0
}









//-------------Neurone------------------
//--------------------------------------

//--------------------------------------
/**
 *  Constructeur de neurone
 */
::Neurone::Neurone(unsigned parNbOutput, unsigned parMyIndex)
{
	for (unsigned c = 0; c < parNbOutput; ++c) {					//Pour toutes les sorties du neurone
		outputPoids_.push_back(Connection());						//on creer une connection
	}
	myIndex_ = parMyIndex;
	ETA = TAUX_ENTRAINEMENT;
	ALPHA = MOMENTUM;
	//numero de neurone dans le layer
}

//--------------------------------------
/**
 *  calcul de la sortien en fonction des entrees
 *  output=f(sum(i*w))
 *  @param parPrevLayer la couche d'avant (qui constitue les entrees)
 */
void ::Neurone::feedForward(const Layer &parPrevLayer)
{
	double sum = 0.0;
	
	for (auto& n : parPrevLayer) {			//pour tout les neurones de la couche d'avant
		sum +=	n.getOutputValue() *					//Sum = S(val_neurone_avant *poids_connection)
		n.outputPoids_[myIndex_].poids_;
	}
	
	outputValue_ = Neurone::fctTransfert(sum);						//on applique à somme la fonction de transfert
}

//--------------------------------------
/**
 *  calcul du gradient de sortie
 *  @param parTargetVal valeur de sortie attendu
 */
void ::Neurone::calcOutputGradients(double parTargetVal)
{
	double delta = parTargetVal - outputValue_;						//ecart entre la theorie et la sortie
	gradient_ = delta * Neurone::fctTransfertDerivee(outputValue_);	//on applique la derivee dela fct de transfert
}

//--------------------------------------
/**
 *  calcul du gradient des couches intermediaires
 *  @param parNextLayer ref vers la couche superieur
 */
void ::Neurone::calcHiddenGradients(const Layer &parNextLayer)
{
	//dow : derivative output weight
	double dow = sumDOW(parNextLayer);								//on calcul le gradient des sorties en fonction des poids
	gradient_ = dow * Neurone::fctTransfertDerivee(outputValue_);	//on applique la derivee de la fct de transfert
}

//--------------------------------------
/**
 *  mise à jour des poids
 *  @param parPrevLayer ref sur la couche d'avant
 */
void ::Neurone::updateInputsPoids(Layer & parPrevLayer)
{
	for (auto& n : parPrevLayer) {			//pour tout les neurones de la couches
		Neurone &neurone = n;
		double ancDeltaPoids = neurone.outputPoids_[myIndex_].deltaPoids_;
		double nouvDeltaPoids =
			(ETA * neurone.getOutputValue() * gradient_ )
			+ (ALPHA * ancDeltaPoids);								//le d(poids) = taux d'entrainement * gradient + momentum * l'ancien delta
		neurone.outputPoids_[myIndex_].deltaPoids_ = nouvDeltaPoids;//update ancien d(poids)
		neurone.outputPoids_[myIndex_].poids_ += nouvDeltaPoids;	//on modifier le poids avec le d(poids)
		
	}
}

//--------------------------------------
/**
 *  donne le tableau de connection du neurone
 *  @param parConnections le tableau de stockage
 */
void ::Neurone::getConnectionsValues(Connections & parConnections) const
{
	parConnections = outputPoids_;
}

//--------------------------------------
void ::Neurone::setConnectionsValues(Connections &parConnections)
{
	outputPoids_ = parConnections;
}

//--------------------------------------
/**
 *  fonction de transfert
 *  @param parSum valeur de la somme
 *  @return valeur de sortie
 */
double ::Neurone::fctTransfert(double parSum)
{
	//std::cout << val << std::endl;
	return tanh(LAMBDA * parSum);
}

//--------------------------------------
/**
 *  derivee de la fonction de transfert
 */
double ::Neurone::fctTransfertDerivee(double par)
{
	//2sech^2(2x)~2-8x^2 en 0
	//return LAMBDA - par * par;
	return 1.0 - pow(tanh(LAMBDA * par), 2.0);
}

//--------------------------------------
/**
 *  Somme des derivee des poids de sortie
 *  @param parNextLayer ref vers la couche supérieur
 *  @return somme des derivee de poids
 */
double ::Neurone::sumDOW(const Layer &parNextLayer) const
{
	double sum = 0.0;
	for (unsigned n = 0; n < parNextLayer.size() - 1; ++n) {		//pour tout les neurones de la couche supérieur - bias
		sum += outputPoids_[n].poids_ * parNextLayer[n].gradient_;	//S(poids_connection * gradient
	}
	return sum;
}










//-------------Network------------------
//--------------------------------------

//int Network::nombreMesure_ = NB_MESURE;

//--------------------------------------
/**
 *  Constructeur du reseau de neurone
 */
::Network::Network(const unsigned int parNbInput, const unsigned int parNbOutput)
{
	error_ = 0.0;
	derniereMoyenneErreur_ = 0.0;
	nombreMesure_ = NB_MESURE;
	constructNetworkRandom(parNbInput,parNbOutput);
	nbLayers_ = layers_.size();
}

//--------------------------------------
/**
 *  Constructeur du reseau de neurone
 */
::Network::Network(const std::vector<unsigned> & parTopologie, const Text & parSave)
{
	error_ = 0.0;
	derniereMoyenneErreur_ = 0.0;
	nombreMesure_ = NB_MESURE;
	
	assert(!parTopologie.empty());									//si la topologie est vide, on arrete
	nbLayers_ = parTopologie.size();
	topologie_ = parTopologie;
	
	if (parSave.compare("") == 0) {
		constructNetworkFromScratch(parTopologie);
	}
	else
	{
		if(constructNetworkFromFile(parSave))
			constructNetworkFromScratch(parTopologie);
	}
	//
	
	
	//	for (auto numLayers = 0; numLayers < nbLayers; ++numLayers) {
	//		layers_.push_back(Layer());
	//		unsigned nbOutput = (numLayers == parTopologie.size() - 1)?
	//		0:
	//		parTopologie[numLayers + 1];
	//		//on ajoute les neurones le le "bias" neurone --> <=
	//		for (auto numNeuron = 0; numNeuron <= parTopologie[numLayers]; ++numNeuron) {
	//			layers_.back().push_back(Neurone(nbOutput,numNeuron));
	//			std::cout << "Ajout du neurone : " << numNeuron << " a la couche : " << numLayers <<  std::endl;
	//		}
	//		layers_.back().back().setOutputValue(1.0);
	//	}
}

//--------------------------------------
void ::Network::constructNetworkFromScratch(const std::vector<unsigned int> &parTopologie)
{
	std::cout << "creation from scratch" << std::endl;
	for (unsigned i = 0; i < parTopologie.size(); ++i) {			//pour toutes les couches
		unsigned nbNeurone = parTopologie[i];						//nombre de neurone dans la couche
		assert(nbNeurone > 0);										//si le nombre de neurone est 0, on arrete
		layers_.push_back(Layer());									//on ajoute la couche au reseau
		Layer &newLayer = layers_.back();							//on pointe vers la derniere couche ajouter
		bool isLastLayer = (i == (parTopologie.size() - 1));		//si cest la derniere couche
		unsigned nbOutput = (isLastLayer)? 0 : parTopologie[i + 1];	//si c'est la derniere on a 0 output, sinon on a i+1
		
		for (unsigned j = 0; j < (nbNeurone + 1); ++j) {			//pour le nombre de neurone + bias
			newLayer.push_back(Neurone(nbOutput, j));				//on creer un neurone
		}
		
		Neurone &biasNeurone = newLayer.back();
		biasNeurone.setOutputValue(1.0);							//on met la valeur du neurone de bias à 0
	}
}

//--------------------------------------
bool ::Network::constructNetworkFromFile(const Text & parSave)
{
	
	std::vector<unsigned> topo;
	inputFile_ = std::unique_ptr<std::ifstream>(new std::ifstream(parSave));
	if (!(inputFile_->good())) {
		std::cout << "fichier inexistant" << std::endl;
		return 1;
	}
	inputFile_->setf(std::ifstream::in);
	//inputFile_->open(parSave);
	
	if (!inputFile_) {
		std::cerr << "probleme a l'ouverture du fichier" << std::endl;
	}
	
	Connections cons;
	double x = 0;
	double y = 0;
	double a = 0;

	Text ligne;
	Text label;
	
	getline(*inputFile_, ligne);
	std::stringstream ss(ligne);
	
	ss >> label;
	
	if(label.compare("topologie:") != 0)
	{
		return 1;
	}
	
	unsigned n;
	while (!ss.eof()) {
		ss >> n;
		topo.push_back(n);
	}
	topo.pop_back();
	
	for (unsigned i = 0; i < topo.size(); ++i) {
		unsigned nbNeurone = topo[i];
		if (nbNeurone == 0) {
			return 1;
		}
		assert(nbNeurone > 0);
		layers_.push_back(Layer());
		Layer &newLayer = layers_.back();
		bool isLastLayer = (i == (topo.size() - 1));
		unsigned nbOutput = (isLastLayer)? 0 : topo[i + 1];
		
		for (unsigned j = 0; j < (nbNeurone + 1); ++j) {
			getline(*inputFile_, ligne);
			ss.clear();
			ss.str(ligne);
			ss >> label;
			if (label.compare("neurone:") == 0) {
				
				ss.ignore();
				ss.ignore();
				ss >> x;
				ss >> x;
				ss >> y;
				
			}
			newLayer.push_back(Neurone(nbOutput, j));
			Neurone& neur = newLayer.back();
			neur.setTauxEntrainement(x);
			neur.setMomentum(y);
			
			for (unsigned n = 0; n < nbOutput; ++n) {
				getline(*inputFile_, ligne);
				ss.clear();
				ss.str(ligne);
				ss >> label;
				if (label.compare("connection:") == 0) {
					
					ss.ignore();
					ss.ignore();
					ss >> a;
					ss >> a;
					
				}
				cons.push_back(Connection(a));
			}
			neur.setConnectionsValues(cons);
			cons.clear();
		}
		
		Neurone &biasNeurone = newLayer.back();
		biasNeurone.setOutputValue(1.0);
	}
	inputFile_->close();
	std::cout << "creation from file" << std::endl;
	return 0;
}

//--------------------------------------
void Network::constructNetworkRandom(const unsigned parNbInput, const unsigned parNbOutput)
{
	topologie_.push_back(parNbInput);
	for (unsigned n = 0; n < (std::rand() % 3); ++n) {
			topologie_.push_back(parNbInput + ( std::rand() % ( 2*parNbInput - parNbInput + 1 ) ));
	}
	topologie_.push_back(parNbOutput);
	constructNetworkFromScratch(topologie_);
}

//--------------------------------------
/**
 *  propagation vers les couches supérieur du reseau
 *  @param parInputValues ref du tableaus des entrees
 */
void Network::feedForward(const T_val & parInputValues)
{
	assert(parInputValues.size() == layers_[0].size() -1);			//si le nombre d'entree ne correspond pas au nb de neurone d'entree
	
	for (unsigned i = 0; i < parInputValues.size(); ++i) {			//pour le nombre d'entree
		layers_[0][i].setOutputValue(parInputValues[i]);			//on met les valeurs dans les neurones d'entree
	}
	
	for (auto layerNb = 1; layerNb < layers_.size(); ++layerNb) {	//pour toutes les couches inetermédiaires
		Layer &currentLayer = layers_[layerNb];
		Layer &prevLayer = layers_[layerNb - 1];
		for (auto n = 0; n < currentLayer.size() - 1; ++n) {		//pour tout les neurones de la couches
			currentLayer[n].feedForward(prevLayer);					//on nourrie la couche
		}
	}
}

//--------------------------------------
/**
 *  "back propagation", on calcul les erreurs, les gradients et on update les poids
 *  @param parTargetValues ref vers le tableau des targets
 */
void Network::backProp(const T_val &parTargetValues)
{
	//calculer l'erreur du réseau avec la RMS (Root Mean Square Error)
	Layer &outputLayer = layers_.back();
	error_ = 0.0;													//erreur = sqrt((d1^2+d2^2+...dn^2)/n)
	
	for (unsigned n = 0; n <  outputLayer.size() - 1; ++n) {		//pour tout les neurone de la couche de sortie - bias
		double delta = static_cast<double>
			(parTargetValues[n]- outputLayer[n].getOutputValue());	//on calcul le delta entre les targets et sortie
		error_ += pow(delta , 2.0);									//on ajoute le carré du delta à l'erreur
	}
	error_ /= static_cast<double>(outputLayer.size() - 1.0);		//on divise l'erreur par la nb de neurone
	error_ = sqrt(error_); //RMS									//et on fait la racine
	
	//moyenne des mesures
	derniereMoyenneErreur_ =
	(derniereMoyenneErreur_ * nombreMesure_ + error_)
	/ (nombreMesure_+ 1.0);
	
	//calcul du gradient de sortie
	for (unsigned n = 0; n < outputLayer.size(); ++n) {				//calcul du gradient de sortie des neurones de sortie
		outputLayer[n].calcOutputGradients(parTargetValues[n]);
	}
	
	//calcul du gradient des couches cachees
	for (unsigned layerNum = static_cast<unsigned>(layers_.size() - 2); layerNum > 0; --layerNum) {
		Layer &hiddenLayer = layers_[layerNum];
		Layer &nextLayer = layers_[layerNum + 1];
		for (unsigned n = 0; n < hiddenLayer.size(); ++n) {
			hiddenLayer[n].calcHiddenGradients(nextLayer);			//calcul du gradient de sortie des neurones intermédiaire
		}
	}
	
	//Pour toutes les couches, de la sortie à la premiere couche cachees on update les poids
	for (unsigned layerNum = static_cast<unsigned>(layers_.size() - 1); layerNum > 0; --layerNum) {
		Layer &layer =		layers_[layerNum];
		Layer &prevLayer =	layers_[layerNum - 1];
		
		for (unsigned n = 0; n < static_cast<unsigned>(layer.size() - 1); ++n) {
			layer[n].updateInputsPoids(prevLayer);					//mise à jour des poids des connections
		}
	}
}

//--------------------------------------
/**
 *  récupération des résultats
 *  @param parResultValues ref tableau de resultat
 */
void Network::getResults(T_val & parResultValues) const
{
	parResultValues.clear();
	
	for (auto n : layers_.back()) {
		parResultValues.push_back(n.getOutputValue());
	}
}

//--------------------------------------
/**
 *  fonction de prédiction
 *  @param parVal ref du tableau d'entree
 *  @return le tableau de resultat
 */
T_val Network::predict(const T_val & parVal)
{
	feedForward(parVal);											//on nourrie le réseau avec les valeurs d'entrée
	T_val resultat;
	getResults(resultat);											//on récupère les résultats de sortie
	return resultat;
}

//--------------------------------------
/**
 *  affiche les poids des neurones du network
 */
void Network::printNeuroneConnectionsPoids() const
{
	Connections Cons;
	for (unsigned i = 0; i < layers_.size() - 1; ++i) {
		for (unsigned j = 0; j < layers_[i].size(); ++j) {
			std::cout << "Neurone [" << i << "][" << j << "]" <<std::endl;
			Neurone Neur = layers_[i][j];
			Neur.getConnectionsValues(Cons);
			for (unsigned n = 0; n < Cons.size() ; ++n) {
				std::cout << "-> [" << i + 1 << "][" << n << "]:(" << Cons[n].poids_ << ")" << std::endl;
			}
		}
	}
}

//--------------------------------------
/**
 *  renvoie la topologie du reseau
 *  @param parTopologie le tableau qui va stocker la topologie
 */
void Network::getNetworkTopologie(Topologie &parTopologie) const
{
	parTopologie = layers_;
}

//--------------------------------------
void Network::getTopologie(std::vector<unsigned> & parTopo) const
{
	parTopo = topologie_;
}

//--------------------------------------
void Network::setNbMesure(int par)
{
	nombreMesure_ = par;
}

//--------------------------------------
void Network::saveInFile()
{
	Connections cons;
	Neurone* neur;
	outputFile_.setf(std::ofstream::out);
	outputFile_.open("save.txt");
	
	if (!outputFile_) {
		std::cerr << "probleme a l'ouverture du fichier" << std::endl;
		return;
	}
	outputFile_ << "topologie: ";
	for (unsigned n = 0; n < layers_.size(); ++n) {
		outputFile_ << layers_[n].size() - 1 << " ";
	}
	outputFile_ << std::endl;
	for (unsigned i = 0; i < layers_.size() - 1; ++i) {
		for (unsigned j = 0; j < layers_[i].size(); ++j) {
			neur = &layers_[i][j];
			outputFile_ << "neurone: " << i << " " << j << " " << neur->getTauxEntrainement() << " " << neur->getMomentum() << std::endl;
			neur->getConnectionsValues(cons);
			for (unsigned k = 0; k < cons.size(); ++k) {
				outputFile_ << "connection: " << i + 1 << " " << k << " " << cons[k].poids_ << std::endl;
			}
		}
	}
	outputFile_.close();
}

//--------------------------------------
Cellule::Cellule()
{
	
}

//--------------------------------------
Systeme::Systeme(unsigned parNbCell)
{
	nbCellule_ = parNbCell;
	std::vector<unsigned> topo;
	trainingData_ = std::unique_ptr<ReadTrainData>(new ia::ReadTrainData("exemples/binaire.txt"));
	trainingData_->getTopologie(topo);
	for (unsigned n = 0; n < nbCellule_; ++n) {
		tabCell_.push_back(Network(topo.front(),topo.back()));
	}
}

//--------------------------------------
void Systeme::trainAllSysteme(T_val& parInputVals, T_val& parTargetVals, T_val& parResultVals)
{
//	std::vector<unsigned> topo;
//	std::vector<Network> tmp;
//	
//	//tant que l'erreur est importante
//	while (*std::min_element(erreurNet_.begin(), erreurNet_.end()) > ERREUR) {
//		//on lance l'entrainement de toutes les cellules
//		for (unsigned n = 0; n < nbCellule_; ++n) {
//			tabCell_[n].getTopologie(topo);
//			training(tabCell_[n], *trainingData_,topo, parInputVals, parTargetVals, parResultVals);
//			erreurNet_.push_back(tabCell_[n].getErreurMoyenne());
//		}
//		
//		tmp = tabCell_;
//		//on choisie les meilleurs
//		//auto it = std::min_element(erreurNet_.begin(), erreurNet_.end());
//		//unsigned i = std::distance(it, erreurNet_.begin());
//	
//	}

	
}

//--------------------------------------











