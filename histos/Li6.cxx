
#include "TRuntimeObjects.h"

#include <iostream>
#include <chrono>
#include <thread>
#include <sstream>
#include <string>
#include <utility>


#include "TH1.h"
#include "TH2.h"
#include "TMath.h"
#include "TRandom.h"
#include "TObject.h"
#include "TFile.h"
#include "TCutG.h"
#include "TPreserveGDirectory.h"
#include "TCagra.h"
#include "TGrandRaiden.h"

#define BAD_NUM -441441

#include "TANLEvent.h"
//#include "GValue.h"

#define PRINT(x) std::cout << #x" = " << x << std::endl
#define STR(x) #x << " = " <<() x

using namespace std;


static string name;
static string dirname;
static stringstream stream;

void MakeCAGRAHistograms(TRuntimeObjects&, TCagra&);
void MakeGrandRaidenHistograms(TRuntimeObjects&, TGrandRaiden&);
void MakeCoincidenceHistograms(TRuntimeObjects&, TCagra&, TGrandRaiden&);
void LoadCuts();

static std::vector<double> acoefs =
{
  25478959.64415743201971054,
  -0.01771996157436579,
  -0.00000375686641008,
  -25478962.08887941017746925,
  339.32639498225580610,
  285.41500265652570079
};
static std::vector<double> bcoefs =
{
  -8.77979136468771948,
  3.57167199982826045,
  144.77038232928563843,
  -102.62252113966886213,
  -1669.00084319835968927,
  1060.28866587772699859,
  0.00281259943988080,
  0.00390660581203093,
  -0.57697427700803183,
  -0.03624075884543333,
  10.73431026156209711,
  -0.86971847190018392,
  0.00002737725798545,
  -0.00000000249027871,
  -0.00140344227013718,
  0.00008863729153273,
  0.01262661852581874,
  -0.00031269175012557
};

static unsigned int xdegree = 2, adegree = 2, ydegree = 1;
std::pair<double,double> raytrace(double x, double a, double y) {
  double sum = 0;
  double count = 0;
  double A = 0;
  double B = 0;
  // dispersive angle raytrace
  for (auto i=0u; i<=xdegree; i++) {
    sum += acoefs[i]*pow(x,i);
    count = i;
  }
  for (auto i=0u; i<=adegree; i++) {
    sum += acoefs[i+count+1]*pow(a,i);
  }
  A=sum;
  sum=count=0;

  // non-dispersive angle raytrace
  for (auto i=0u; i<= xdegree; i++) {
    double sum2 = 0;
    for (auto j=0u; j<= adegree; j++) {
      double sum3 = 0;
      for (auto k=0u; k<= ydegree; k++) {
        sum3 += bcoefs[count]*pow(y,k);
        count++;
      }
      sum2 += sum3*pow(a,j);
    }
    sum += sum2*pow(x,i);
  }
  B=sum;

  return std::pair<double,double>(A,B);
}

// ----------------------------------------------------------------------
// extern "C" is needed to prevent name mangling.
// The function signature must be exactly as shown here,
//   or else bad things will happen.
extern "C"
void MakeHistograms(TRuntimeObjects& obj) {
  LoadCuts();

  TCagra* cagra = obj.GetDetector<TCagra>();
  TGrandRaiden* gr = obj.GetDetector<TGrandRaiden>();

  if (gr) {
    MakeGrandRaidenHistograms(obj,*gr);
  }
  if (cagra) {
    MakeCAGRAHistograms(obj,*cagra);
  }
  if (cagra && gr) {
    MakeCoincidenceHistograms(obj, *cagra, *gr);
  }

}
// ----------------------------------------------------------------------

void PoleZeroHistos(TRuntimeObjects& obj, TCagraHit& core_hit, string local_dirname = "") {

  auto boardid = core_hit.GetBoardID();
  auto chan = core_hit.GetChannel();



  stream.str("");
  stream << "Ge_PZ_" << boardid << "_" << chan;
  obj.FillHistogram(local_dirname, stream.str(),5000,0,10000,(-1)*core_hit.GetCorrectedEnergy(0));



  Double_t baseline = core_hit.GetPreRise()/TANLEvent::GetShapingTime();
  baseline = core_hit.GetSampledBaseline();

  //if ((boardid==114) || (boardid==116) ||  (boardid==117) || (boardid==118) || (boardid==119) || (boardid==120)) {
  if (boardid==118 || boardid==119) {

    stream.str("");
    stream << "SampledBaseline[Q]_" << boardid << "_" << chan;
    obj.FillHistogram(local_dirname, stream.str(),2000,0,10000,(-1)*core_hit.Charge(),750,0,0,baseline);

    stream.str("");
    stream << "SampledBaseline[E_pzcor]_" << boardid << "_" << chan;
    obj.FillHistogram(local_dirname, stream.str(),2000,0,10000,(-1)*core_hit.GetCorrectedEnergy(baseline),750,0,0,baseline);
  }

    stream.str("");
    stream << "Ge_PZ_SampledBL_" << boardid << "_" << chan;
    obj.FillHistogram(local_dirname, stream.str(),5000,0,10000,(-1)*core_hit.GetCorrectedEnergy(baseline));



    baseline = core_hit.GetBaseSample();

    if (boardid==117 || boardid==118 || boardid==119) {
    stream.str("");
    stream << "BaseSample[Q]_" << boardid << "_" << chan;
    obj.FillHistogram(local_dirname, stream.str(),2000,0,10000,(-1)*core_hit.Charge(),750,7500,9000,baseline);

    stream.str("");
    stream << "BaseSample[E_pzcor]_" << boardid << "_" << chan;
    obj.FillHistogram(local_dirname, stream.str(),2000,0,10000,(-1)*core_hit.GetCorrectedEnergy(baseline),750,7500,9000,baseline);

    }
    stream.str("");
    stream << "Ge_PZ_BaseSample_" << boardid << "_" << chan;
    obj.FillHistogram(local_dirname, stream.str(),5000,0,10000,(-1)*core_hit.GetCorrectedEnergy(baseline));



    baseline = core_hit.GetTraceBaseline();
    if (boardid==118 || boardid==119) {
    stream.str("");
    stream << "TraceBL[Q]_" << boardid << "_" << chan;
    obj.FillHistogram(local_dirname, stream.str(),2000,0,10000,(-1)*core_hit.Charge(),750,7500,9000,baseline);

    stream.str("");
    stream << "TraceBL[E_pzcor]_" << boardid << "_" << chan;
    obj.FillHistogram(local_dirname, stream.str(),2000,0,10000,(-1)*core_hit.GetCorrectedEnergy(baseline),750,7500,9000,baseline);

    }
    stream.str("");
    stream << "Ge_PZ_AsymBLTrace_" << boardid << "_" << chan;
    obj.FillHistogram(local_dirname, stream.str(),5000,0,10000,(-1)*core_hit.GetCorrectedEnergy(baseline));

    //std::cout << core_hit.GetBaseSample() << " " << core_hit.GetTraceBaseline() << std::endl;

}


void PileUp(TRuntimeObjects& obj, TCagraHit& core_hit) {
  dirname = "PileUp";

  auto flags = core_hit.GetFlags();
  auto pileup = TANLEvent::PileUpFlag(flags);
  if (pileup) {
    obj.FillHistogram(dirname,"Summary",10,1,10,3);
    PoleZeroHistos(obj,core_hit,dirname);
  } else {
    obj.FillHistogram(dirname,"Summary",10,1,10,7);
    PoleZeroHistos(obj,core_hit,"NoPileUp");
  }
}


void MakeCAGRAHistograms(TRuntimeObjects& obj, TCagra& cagra) {

  bool pulser_event = false;

  for (auto& core_hit : cagra) {

    if (core_hit.GetSystem() == 'P') {
      pulser_event = true;
    }

  }

  for (auto& core_hit : cagra) {

    //TChannel::Get(core_hit.Address())->Print();
    //std::cin.get();


    PileUp(obj,core_hit);

    auto boardid = core_hit.GetBoardID();
    auto chan = core_hit.GetChannel();

    if (core_hit.GetSystem() == 'P') {
      obj.FillHistogram("Pulser","Pulser",100,-10000,10000,core_hit.Charge());
    }

    // core_hit.DrawTrace(0);
    // std::cin.get();

    stream.str("");
    if ((boardid==114) || (boardid==116) ||  (boardid==117) || (boardid==118) || (boardid==119) || (boardid==120)) {
      stream << "Ge_" << boardid << "_" << chan;
      obj.FillHistogram("CAGRA_Raw", stream.str(),2500,0,5000,(-1)*core_hit.Charge());
    } else {
      stream << "Ge_" << boardid << "_" << chan;
      obj.FillHistogram("CAGRA_Raw", stream.str(),2500,0,5000,core_hit.Charge());
    }
    if (pulser_event != true)
    {
      stream.str("");
      stream << "Ge_antiP_" << boardid << "_" << chan;
      obj.FillHistogram("CAGRA_Raw", stream.str(),2500,0,5000,core_hit.Charge());
    }

    for (auto& segment : core_hit) {
      stream.str("");
      stream << "Ge_" << boardid << "_" << segment.GetChannel();
      obj.FillHistogram("CAGRA_Raw",stream.str(),2500,0,5000,segment.Charge());
    }

    stream.str("");
    stream << "Ge_" << boardid << "_" << chan;
    obj.FillHistogram("CAGRA_Calibrated",stream.str(),2500,-5000,5000,core_hit.GetEnergy());
    for (auto& segment : core_hit) {
      stream.str("");
      stream << "Ge_" << boardid << "_" << segment.GetChannel();
      obj.FillHistogram("CAGRA_Calibrated",stream.str(),2500,-5000,5000,segment.GetEnergy());
    }


    // new exponential base line correction


    stream.str("");
    stream << "Ge_FitExpBLCorr_" << boardid << "_" << chan;
    // auto Eexpcorr = core_hit.GetBaselineExpCorrFast();
    // if (Eexpcorr) {
    //   obj.FillHistogram("CAGRA_Corrections", stream.str(),5000,0,10000,Eexpcorr);
    // }

    auto trace = core_hit.GetTrace();


    // static int counter=0;
    // if (counter%999==0) {
    //   if(trace->size() >10) {
    //     core_hit.DrawTrace(0);
    //     std::cin.get();
    //   }
    // }

    if(trace->size() > 20) {
      size_t size = 20;
      auto prerise = 0.0;
      for (auto i=0u; i<size; i++) {
        prerise+=(*trace)[i];
      }
      prerise /= size;
      stream.str("");
      stream << "Ge_TraceHeight_" << boardid << "_" << chan;
      obj.FillHistogram("CAGRA_Raw", stream.str(),2500,0,5000,(-1)*core_hit.GetTraceHeight(size));
      stream.str("");
      stream << "Prerise[TraceHeight]" << boardid << "_" << chan;
      obj.FillHistogram("CAGRA_Baseline", stream.str(),
                        2500,-1000,4000,(-1)*core_hit.GetTraceHeight(size),
                        2000,0,10000,prerise);
    }


    // skip polezero for LaBr3 system
    PoleZeroHistos(obj,core_hit,"CAGRA_Corrections");



    //core_hit.DrawTraceSamples(0);
    //std::cin.get();

    static ULong_t first_ts = 0;
    if (first_ts <= 1e6){  first_ts = core_hit.Timestamp(); std::cout << first_ts << std::endl; }
    else {
      obj.FillHistogram("NumEvents","cagra_hits_time",1000,0,8000,(core_hit.Timestamp()-first_ts)*10/1.0e9);
    }



    // Manual trace analysis
    if (false && core_hit.GetDetnum() <= 104) {
      //LaBr3 hit, do custom trace analysis
      stream.str(""); stream << "LaBr3_traces_" << boardid << "_" << chan;
      auto labr_E = core_hit.GetTraceEnergy(0,57);
      obj.FillHistogram("CAGRA_Raw",stream.str(),10000,0,10000,labr_E);
    }

  }

}
void MakeGrandRaidenHistograms(TRuntimeObjects& obj, TGrandRaiden& gr) {

  std::function<void(std::string)> fp_corrections;

  for (auto& hit : gr) {

    auto& rcnp = hit.GR();


    // add myself for LaBr test energy

    double Egamma=0;
    for (auto const& labr_hit : hit.GetLaBr()) {
      int channum = labr_hit.channel;

      Egamma = labr_hit.width*0.00190458-1.27177;
      stream.str(""); stream << "LaBr_cal_" << channum;
      obj.FillHistogram("GR_LABR",stream.str(), 2500,0.,5.,Egamma);

    }


    if (rcnp.GR_MYRIAD(0) != BAD_NUM) {
      obj.FillHistogram("Timing","MyriadTimestamp",10000,1e9,5e12,hit.GetTimestamp());
    }

    static ULong_t prev_ts = 0;
    if (prev_ts) {
      obj.FillHistogram("Timing","GR_EventPeriod",5000,100,50000,hit.GetTimestamp()-prev_ts);
    }
    prev_ts = hit.GetTimestamp();

    auto rf = rcnp.GR_RF(0);
    if (rf != BAD_NUM) {
      obj.FillHistogram("GR","GR_RF",1000,0,0,rf);
    }


    // X, A, Y, B, RF, DE1, DE2
    // X[A],X[Y],X[B],X[RF],DE1[X],DE2[X]
    // Y[A],Y[B],Y[RF],DE1[Y],DE2[Y]
    // A[B], A[RF], DE1[A], DE2[A]
    // B[RF], DE1[B], DE2[B]
    // DE1[RF], DE2[RF], DE1[DE2]

    obj.FillHistogram("GR","RayID",64,-16,48, rcnp.GR_RAYID(0));
    if (rcnp.GR_RAYID(0) == 0) { // if track reconstruction successfull
      obj.FillHistogram("GR","X",1200,-600,600, rcnp.GR_X(0));
      obj.FillHistogram("GR","X_cal",1000,0,20, rcnp.GR_X(0)*0.01074+6.872);
      obj.FillHistogram("GR","Y",200,-100,100, rcnp.GR_Y(0));
      obj.FillHistogram("GR","A",100,-1,1, rcnp.GR_TH(0)); // need to learn
      obj.FillHistogram("GR","B",100,-1,1, rcnp.GR_PH(0)); // from hist.def
      obj.FillHistogram("GR","A[X]",1200,-600,600,rcnp.GR_X(0),1000,-1,1,rcnp.GR_TH(0));
      obj.FillHistogram("GR","Y[A]",600,-0.15,0.15,rcnp.GR_TH(0),500,-50,50,rcnp.GR_Y(0));
      obj.FillHistogram("GR","Y[B]",500,-0.1,0.1,rcnp.GR_PH(0),500,-50,50,rcnp.GR_Y(0));


      dirname = "GR_new";
      obj.FillHistogram(dirname,"X[A]",300,-0.15,0.15,rcnp.GR_TH(0),1200,-600,600,rcnp.GR_X(0));
      obj.FillHistogram(dirname,"X[Y]",200,-100,100,rcnp.GR_Y(0),1200,-600,600,rcnp.GR_X(0));
      obj.FillHistogram(dirname,"X[B]",250,-0.1,0.1,rcnp.GR_PH(0),1200,-600,600,rcnp.GR_X(0));
      //obj.FillHistogram(dirname,"X[RF]",500,0,0,rcnp.GR_RF(0),1200,-600,600,rcnp.GR_X(0));
      obj.FillHistogram(dirname,"RF[A]",1000,-1,1,rcnp.GR_TH(0),500,0,0,rcnp.GR_RF(0));

      auto rf_Acor = rcnp.GR_RF(0)-(-1914.5*rcnp.GR_TH(0));
      obj.FillHistogram(dirname,"RF_Acor[A]",1000,-1,1,rcnp.GR_TH(0),500,0,0,rf_Acor);
      obj.FillHistogram(dirname,"RF[X]",1200,-600,600,rcnp.GR_X(0),500,0,0,rcnp.GR_RF(0));
      obj.FillHistogram(dirname,"RF_Acor[X]",1200,-600,600,rcnp.GR_X(0),500,0,0,rf_Acor);
      auto rf_Acor_Xcor = rf_Acor - (0.17205*rcnp.GR_X(0));
      obj.FillHistogram(dirname,"RF_Acor_Xcor[X]",1200,-600,600,rcnp.GR_X(0),500,0,0,rf_Acor_Xcor);
      obj.FillHistogram(dirname,"DE1[X]",1200,-600,600,rcnp.GR_X(0),2000,0,2000, hit.GetMeanPlastE1());
      obj.FillHistogram(dirname,"DE2[X]",1200,-600,600,rcnp.GR_X(0),2000,0,2000, hit.GetMeanPlastE2());

      obj.FillHistogram(dirname,"Y[A]",300,-0.15,0.15,rcnp.GR_TH(0),200,-100,100,rcnp.GR_Y(0));
      obj.FillHistogram(dirname,"Y[B]",250,-0.1,0.1,rcnp.GR_PH(0),200,-100,100,rcnp.GR_Y(0));
      obj.FillHistogram(dirname,"Y[X]",1200,-600,600,rcnp.GR_X(0),200,-100,100,rcnp.GR_Y(0));
      obj.FillHistogram(dirname,"Y[X]",1200,-600,600,rcnp.GR_X(0),200,-100,100,rcnp.GR_Y(0));
      obj.FillHistogram(dirname,"Y[X]",1200,-600,600,rcnp.GR_X(0),200,-100,100,rcnp.GR_Y(0));
      obj.FillHistogram(dirname,"Y[X]",1200,-600,600,rcnp.GR_X(0),200,-100,100,rcnp.GR_Y(0));

      obj.FillHistogram(dirname,"A[RF]",500,700,1200,rf,1000,-1,1, rcnp.GR_TH(0));



      obj.FillHistogram(dirname,"DE1[RF]",1000,0,0,rf,2000,0,2000, hit.GetMeanPlastE1());
      obj.FillHistogram(dirname,"DE2[RF]",1000,0,0,rf,2000,0,2000, hit.GetMeanPlastE2());
      obj.FillHistogram(dirname,"DE1[RF_Acor_Xcor]",500,0,0,rf_Acor_Xcor,1000,0,2000, hit.GetMeanPlastE1());
      obj.FillHistogram(dirname,"DE2[RF_Acor_Xcor]",500,0,0,rf_Acor_Xcor,1000,0,2000, hit.GetMeanPlastE2());
      obj.FillHistogram(dirname,"DE1[dE2]",2000,0,2000, hit.GetMeanPlastE2(),2000,0,2000, hit.GetMeanPlastE1());
      obj.FillHistogram(dirname,"dE1[A]",1000,-1,1, rcnp.GR_TH(0),2000,0,2000, hit.GetMeanPlastE1());
      obj.FillHistogram(dirname,"dE2[A]",1000,-1,1, rcnp.GR_TH(0),2000,0,2000, hit.GetMeanPlastE2());






      static std::vector<double> xcuts = { -450, 50, 300 };
      // if (xcuts.size()==0) {
      //   double start = -600;
      //   while (true) {
      //     xcuts.push_back(start);
      //     start += 100;
      //     if (start==600) { break; }
      //   }
      // }

      static std::vector<std::vector<std::pair<double,double>>> acuts = { {{-0.069,-0.044},{-0.044,-0.024},{-0.024,-0.003},{-0.001,0.019},{0.02,0.04}  }, { }, { } };
      static std::vector<std::vector<double>> acentroid = {{-0.054,-0.034, -0.1315, 0.00876, 0.0283},{-0.02858,-0.008848,0.01126,0.03202,0.05137},{-0.014,0.00498,0.026,0.0452,0.064}};
      auto x = rcnp.GR_X(0);
      double xwidth = 75;
      auto afp = rcnp.GR_TH(0);
      auto i=0u;
      for (auto& cut : xcuts) {
        if (x < cut+xwidth && x >= cut) {
          stream.str(""); stream << "_x[" << cut << "," << cut+xwidth << ")";
          obj.FillHistogram(dirname,"Y[A]"+stream.str(),300,-0.15,0.15,rcnp.GR_TH(0),200,-100,100,rcnp.GR_Y(0));

          auto& local_acuts = acuts[i];
          for (auto& acut : local_acuts) {
            if (afp < acut.second && afp >= acut.first) {
              stringstream stream2; stream2.str("");
              stream2 << "_a[" << acut.first << "," << acut.second << ")";
              obj.FillHistogram(dirname,"Y"+stream.str()+stream2.str(),200,-100,100, rcnp.GR_Y(0));
            }
          }


        }
        i++;
      }



      // a corrected for x position

      //auto acor = - ((3.41887e-05)*x  +  (4.82521e-08)*x*x);
      auto acor = -5.0043e-05*x;

      afp += acor;
      obj.FillHistogram(dirname,"A[X]corr",1200,-600,600,rcnp.GR_X(0),1000,-1,1,afp);



      dirname = "Target";
      auto ata = 316.129258*afp + -2.443; // mrad
      ata = ata/1000*180/TMath::Pi();
      obj.FillHistogram(dirname,"A[xfp]",1200,-600,600,rcnp.GR_X(0),1000,-1.5,1.5,ata);
      obj.FillHistogram(dirname,"A",1000,-1.5,1.5,ata);





      // raytracing
      double A=0,B=0;
      std::tie(A,B) = raytrace(rcnp.GR_X(0),rcnp.GR_TH(0),rcnp.GR_Y(0));
      obj.FillHistogram(dirname,"B[A]",500,0,0,A,500,0,0,B);

      for (auto& cut : xcuts) {
        if (x < cut+xwidth && x >= cut) {
          stream.str(""); stream << "_x[" << cut << "," << cut+xwidth << ")";
          obj.FillHistogram(dirname,"B[A]"+stream.str(),500,0,0,A,500,0,0,B);
        }
      }



    }

    if (rcnp.GR_ADC()) {
      auto& adc = *rcnp.GR_ADC();
      for (int i=0; i<4; i++) {
        stream.str(""); stream << "GR_ADC" << i;
        obj.FillHistogram("GR",stream.str().c_str(), 1000,0,2000, adc[i]);
      }
    }


    for (auto const& labr_hit : hit.GetLaBr()) {

      int channum = labr_hit.channel;
      stream.str(""); stream << "LaBrLeading" << channum;
      obj.FillHistogram("GR", stream.str(), 10000,-40000, 40000, labr_hit.qtc_le);

      stream.str(""); stream << "LaBr" << channum << "_LE[LaBr_width]";
      obj.FillHistogram("GR", stream.str(), 1000, -5000, 15000, labr_hit.width, 1000,-40000, 40000, labr_hit.qtc_le);


      stream.str(""); stream << "LaBrWidth" << channum;
      obj.FillHistogram("GR", stream.str(), 10000, -5000, 15000, labr_hit.width);

      if (rcnp.GR_X(0) != BAD_NUM) {
        obj.FillHistogram("GR","X_LaBr",1200,-600,600,rcnp.GR_X(0),10000,-5000,15000,labr_hit.width);
      }

      /* cut on prompt timing peak */ // Needs to be updated for campaign !!!!!!!!!!!
//       if (false && labr_hit.qtc_le>=-4000 && labr_hit.qtc_le <=-3250) {
      if ((labr_hit.qtc_le>=-4000) && (labr_hit.qtc_le<=-3000)) {

        obj.FillHistogram("GR_Prompt","RayID",64,-16,48, rcnp.GR_RAYID(0));
        if (rcnp.GR_RAYID(0) == 0) { // if track reconstruction successfull
          obj.FillHistogram("GR_Prompt","GR_X",1200,-600,600, rcnp.GR_X(0));
          obj.FillHistogram("GR_Prompt","GR_Y",200,-100,100, rcnp.GR_Y(0));
          obj.FillHistogram("GR_Prompt","GR_Theta",100,-1,1, rcnp.GR_TH(0));
          obj.FillHistogram("GR_Prompt","GR_Phi",100,-1,1, rcnp.GR_PH(0));
          obj.FillHistogram("GR_Prompt","X_TH",1200,-600,600,rcnp.GR_X(0),1000,-1,1,rcnp.GR_TH(0));

          obj.FillHistogram("GR_Prompt","GR_Theta_Phi",100,-1,1, rcnp.GR_TH(0),100,-1,1, rcnp.GR_PH(0));
          obj.FillHistogram("GR_Prompt","GR_X_Y",1200,-600,600, rcnp.GR_X(0),200,-100,100, rcnp.GR_Y(0));

        }

        stream.str(""); stream << "LaBrWidth_LEGate" << channum;
        obj.FillHistogram("GR_Prompt",stream.str(), 10000, -5000, 15000, labr_hit.width);

        stream.str(""); stream << "X_LaBrWidth" << channum;
        obj.FillHistogram("GR_Prompt",stream.str(),1200,-600,600,rcnp.GR_X(0),10000,-5000,15000,labr_hit.width);

      }

    }


  }

}
void MakeCoincidenceHistograms(TRuntimeObjects& obj, TCagra& cagra, TGrandRaiden& gr) {

  bool pulser_event = false;

  for (auto& core_hit : cagra) {

    if (core_hit.GetSystem() == 'P') {
      pulser_event = true;
    }

  }



  for (auto& hit : gr) {

    auto& rcnp = hit.GR();
    auto grtime = hit.GetTimestamp();

    auto x = rcnp.GR_X(0);
    auto Ex = x*0.01074+6.872; // calibrated for 12C @ commissioning on 04.10.16

    // coincidence rate
    static ULong_t first_timestamp = grtime;
    if (first_timestamp) {
      auto rate = (grtime-first_timestamp)/1e8;
      //cout << grtime << " " << first_timestamp << endl;
      obj.FillHistogram("COIN","Rate",3000,0,30000, rate);
    }

    // coincidence time difference
    for (auto& core_hit : cagra) {

      auto cagratime = core_hit.Timestamp();
      auto tdiff = cagratime-grtime;
      auto boardid = core_hit.GetBoardID();
      auto channel = core_hit.GetChannel();

      obj.FillHistogram("COIN","Diff_CAGRA_GR", 1000,-500,1500,cagratime-grtime);
      if (pulser_event!=true) {
        obj.FillHistogram("COIN","Diff_CAGRA_GR_noPulser", 1000,-500,1500,cagratime-grtime);
      }

      stream.str("");
      stream << "TimeDiff_" << boardid << "_" << channel;
      obj.FillHistogram("COIN",stream.str().c_str(),1000,-500,1500,tdiff);

      // gate on prompt peak from time diff of the timestamps of CAGRA & GR
      if ( (tdiff > 240) && (tdiff < 255) ){

        stream.str("");
        stream << "Ge_"<< boardid << "_" << channel;
	if ((boardid==114) || (boardid==116) ||  (boardid==117) || (boardid==118) || (boardid==119) ||(boardid==120)) {
	  obj.FillHistogram("COIN_Raw",stream.str(),500,0,10000,(-1)*core_hit.Charge());
        }
        stream.str("");
        stream << "Ge_" << boardid << "_" << channel;
        obj.FillHistogram("COIN_Calibrated",stream.str(),10000,0,10000,core_hit.GetEnergy());

        for (auto& segment : core_hit) {
          stream.str("");
          stream << "Ge_" << boardid << "_" << segment.GetChannel();
          obj.FillHistogram("COIN_Calibrated",stream.str(),10000,0,10000,segment.GetEnergy());
        }

        stream.str("");
        stream << "Ex_Ge_" << boardid << "_" << channel;
        obj.FillHistogram("COIN_Calibrated",stream.str(),500,0,20,Ex,5000,0,10000,(-1)*core_hit.GetEnergy());

      if ((boardid==114) || (boardid==116) ||  (boardid==117) || (boardid==118) || (boardid==119) ||(boardid==120)) {
        stream << "Ge_PZ_AsymBL_" << boardid << "_" << channel;
        obj.FillHistogram("COIN_Calibrated", stream.str().c_str(),5000,0,10000,(-1)*core_hit.GetCorrectedEnergy(core_hit.GetBaseSample()));

        stream.str("");
        stream << "Ex_CAGRACorrected_" << boardid << "_" << channel;
        obj.FillHistogram("COIN_Calibrated", stream.str().c_str(),500,0,20,Ex,10000,0,10000,(-1)*core_hit.GetCorrectedEnergy(core_hit.GetBaseSample()));

      }

      }

    }


  }



}


void LoadCuts() {


}