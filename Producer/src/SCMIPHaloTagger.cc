#include "../interface/SCMIPHaloTagger.h"

#include "RecoEcal/EgammaCoreTools/interface/EcalClusterTools.h" 
#include "DataFormats/EcalDetId/interface/EBDetId.h"
#include "DataFormats/EcalRecHit/interface/EcalRecHit.h"

#include <TMath.h>

void
SCMIPHaloTagger::setup(const edm::ParameterSet& conf)
{
  yRangeFit_             = conf.getParameter<double>("YRangeFit");
  xRangeFit_             = conf.getParameter<double>("XRangeFit");
  residualWidthEnergy_   = conf.getParameter<double>("ResidualWidth");
  haloDiscThreshold_     = conf.getParameter<double>("HaloDiscThreshold");
}

void
SCMIPHaloTagger::fillMIPVariables(const reco::SuperCluster* sc,
                                  EcalRecHitCollection const& ebHits,
                                  reco::Photon::MIPVariables& mipId)
{
  bool debug_= false;

  if(debug_)std::cout<<" inside MipFitTrail "<<std::endl;
  //getting them from cfi config

  //initilize them here
  double m = yRangeFit_ / xRangeFit_; //slope of the lines which form the cone around the trail

  //first get the seed cell index
  int seedIEta = -999;
  int seedIPhi = -999;
  double seedE = -999.; 

  //get seed propert.
  getSeedHighestE(sc,
                  ebHits,
                  seedIEta,
                  seedIPhi,
                  seedE);

  if(debug_)std::cout<<"Seed E ="<<seedE<<"  Seed ieta = "<<seedIEta<<"   Seed iphi ="<< seedIPhi<<std::endl;

  //create some vector and clear them
  std::vector<int> ieta_cell;
  std::vector<int> iphi_cell;
  std::vector<double> energy_cell;

  ieta_cell.clear();
  iphi_cell.clear();
  energy_cell.clear();

  int ietacell =    0;
  int iphicell =    0;
  int kArray   =    0;
  
  double energy_total = 0.;
  int delt_ieta    =0;
  int delt_iphi    =0;

  for(auto& hit : ebHits) {
    EBDetId dit = hit.detid();

    iphicell = dit.iphi();
    ietacell = dit.ieta();
    
    if(ietacell < 0)
      ietacell++;

    //Exclude all cells within +/- 5 ieta of seed cell 
    if(TMath::Abs(ietacell - seedIEta) >= 5  &&  hit.energy() > 0.) {

      delt_ieta = ietacell - seedIEta;
      delt_iphi = iphicell - seedIPhi;

      //Phi wrapping inclusion
      if(delt_iphi > 180){ delt_iphi = delt_iphi - 360; }
      if(delt_iphi < -180){ delt_iphi = delt_iphi + 360; }

      //Condition to be within the cones
      if( ( (delt_iphi >= (m*delt_ieta)) && (delt_iphi <= (-m*delt_ieta)) ) ||
          ( (delt_iphi <= (m*delt_ieta)) && (delt_iphi >= (-m*delt_ieta)) ) 
          ) {
        ieta_cell.push_back(delt_ieta);
        iphi_cell.push_back(delt_iphi);
        energy_cell.push_back(hit.energy());
        energy_total += hit.energy();
        kArray++;
      }

    }//within cerntain range of seed cell

  }//loop voer hits



  //Iterations for imporovements

  int Npoints         =  0; 
  int throwaway_index = -1;
  double chi2(0.);
  double eT          = 0.;         
  double hres        = 99999.;

  //some tmp local variale
  double Roundness_ =999.;
  double Angle_     =999.;
  double halo_disc_ =0.;

  Npoints = kArray;

  if(debug_)std::cout<<" starting npoing = "<<Npoints<<std::endl;

  //defined some variable for iterative fitting the mip trail line  
  double  res    = 0.0;                            
  double  res_sq = 0.0;                            
  double  wt     = 0.0;                            
  double  sx     = 0.0;    
  double  sy     = 0.0;                                   
  double  ss     = 0.0;                            
  double  sxx    = 0.0;                            
  double  sxy    = 0.0;                            
  double  delt   = 0.0;
  double  a1     = 0.0;
  double  b1     = 0.0;                            
  double  m_chi2 = 0.0;                            
  double  etot_cell=0.0;   


  //start Iterations
  for(int it=0; it < 200 && hres >(5.0*residualWidthEnergy_); it++) {  //Max iter. is 200


    //Throw away previous highest residual, if not first loop
    if (throwaway_index!=-1) {
      ieta_cell.erase(ieta_cell.begin()+throwaway_index);
      iphi_cell.erase(iphi_cell.begin()+throwaway_index);
      energy_cell.erase(energy_cell.begin()+throwaway_index);
      Npoints--;
    }
           

    //Lets Initialize them first for each iteration 
    res    = 0.0;
    res_sq = 0.0;
    wt     = 0.0; 
    sx     = 0.0;
    sy     = 0.0; 
    ss     = 0.0;
    sxx    = 0.0;
    sxy    = 0.0;
    delt   = 0.0; 
    a1     = 0.0;
    b1     = 0.0;
    m_chi2 = 0.0;
    etot_cell=0.0;
     
 
    //Fit the line to trail 
    for(int j=0; j<Npoints; j++) {  
      wt = 1.0;
      ss += wt;
      sx += ieta_cell[j]*wt;                               
      sy += iphi_cell[j]; 
      sxx += ieta_cell[j]*ieta_cell[j]*wt;
      sxy += ieta_cell[j]*iphi_cell[j]*wt;
    }
       
    delt =  ss*sxx - (sx*sx);
    a1 = ((sxx*sy)-(sx*sxy))/delt;   // INTERCEPT
    b1 = ((ss*sxy)-(sx*sy))/delt;    // SLOPE


    double highest_res   = 0.;
    int    highres_index = 0;
                         

    for(int j=0; j<Npoints; j++) {                
      res = 1.0*iphi_cell[j] - a1 - b1*ieta_cell[j];
      res_sq = res*res;

      if(TMath::Abs(res) > highest_res)
        {             
          highest_res = TMath::Abs(res);
          highres_index = j;
        }             

      m_chi2 += res_sq;
      etot_cell += energy_cell[j];

    }                
           
              
    throwaway_index = highres_index;
    hres            = highest_res;

    chi2 = m_chi2 /((Npoints-2));
    chi2 = chi2/(residualWidthEnergy_ * residualWidthEnergy_);
    eT   = etot_cell;  

  }//for loop for iterations

  if(debug_)std::cout<<"hres = "<<hres<<std::endl;

  //get roundness and angle for this photon candidate form EcalClusterTool
  std::vector<float> showershapes_barrel = EcalClusterTools::roundnessBarrelSuperClusters(*sc, ebHits);

  Roundness_ = showershapes_barrel[0];
  Angle_     = showershapes_barrel[1];

  if(debug_)std::cout<<" eTot ="<<eT<<"     Rounness = "<<Roundness_<<"    Angle_  "<<Angle_ <<std::endl; 

  //get the halo disc variable
  halo_disc_ = 0.;
  halo_disc_ = eT/(Roundness_* Angle_);


  ///Now Filll the FitResults vector
  mipId.mipChi2 = chi2; //chi2
  mipId.mipTotEnergy = eT;   //total energy
  mipId.mipSlope = b1;   //slope
  mipId.mipIntercept = a1;   //intercept
  mipId.mipNhitCone = Npoints;        //nhit in cone
  mipId.mipIsHalo = (halo_disc_ > haloDiscThreshold_);

  if(debug_)std::cout<<"Endof MIP Trail: halo_dic= "<<halo_disc_<<"   nhitcone ="<< mipId.mipNhitCone <<"  isHalo= "<<mipId.mipIsHalo<<std::endl;
  if(debug_)std::cout<<"Endof MIP Trail: Chi2  = "<<chi2<<"  eT= "<<eT<<" slope = "<<b1<<"  Intercept ="<<a1<<std::endl;
}

//get the seed crystal index
void
SCMIPHaloTagger::getSeedHighestE(const reco::SuperCluster* sc,
                                 EcalRecHitCollection const& Brechit,
                                 int &seedIeta,
                                 int &seedIphi,
                                 double &seedEnergy)
{

  bool debug_= false;

  if(debug_)std::cout<<"Inside GetSeed"<<std::endl;
  //Get the Seed
  double SeedE    = -999.;

  //initilaze them here
  seedIeta   = -999;
  seedIphi   = -999;
  seedEnergy = -999.;

  std::vector< std::pair<DetId, float> >  PhotonHit_DetIds = sc->hitsAndFractions();
  int ncrys   = 0;

  std::vector< std::pair<DetId, float> >::const_iterator detitr;
  for(detitr = PhotonHit_DetIds.begin(); detitr != PhotonHit_DetIds.end(); ++detitr) {

    if (((*detitr).first).det() == DetId::Ecal && ((*detitr).first).subdetId() == EcalBarrel) {
      EcalRecHitCollection::const_iterator j= Brechit.find(((*detitr).first));
      EcalRecHitCollection::const_iterator thishit;

      if ( j!= Brechit.end())  thishit = j;
      if ( j== Brechit.end()){ continue;}
	      

      EBDetId detId  = (EBDetId)((*detitr).first);

      double crysE = thishit->energy();

      if (crysE > SeedE)
        {SeedE = crysE;                     
          seedIeta    = detId.ieta();
          seedIphi    = (detId.iphi());
          seedEnergy  = SeedE;

          if(debug_)std::cout<<"Current max Seed = "<<SeedE<<"   seedIphi = "<<seedIphi<<"  ieta= "<<seedIeta<<std::endl;

        }

      ncrys++;


    }//check if in Barrel
        
  }//loop over EBrechits cells

  if(debug_)std::cout<<"End of  GetSeed"<<std::endl;

} 
