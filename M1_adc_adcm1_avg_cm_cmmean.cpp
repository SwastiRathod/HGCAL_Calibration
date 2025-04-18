#include <iostream>
#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include "TCanvas.h"
#include "TF1.h"
#include "TLegend.h"
#include "TStyle.h"
#include <memory>

void plot_ADC_step1() {
    TFile *file = new TFile("ADC_CM_extracted_branches.root");
    TTree *tree = (TTree*)file->Get("Events");

    const int N_HGC = 444;
    const int N_HGC_CM = 24;
    
    Int_t nHGC;
    Float_t         HGC_adc[N_HGC];
 //   Float_t         HGC_adcm1[N_HGC];    //remove comments for other steps when u need adc adcm
 //   Float_t         HGCCM_cm[N_HGC_CM];

     tree->SetBranchAddress("nHGC", &nHGC);
     tree->SetBranchAddress("HGC_adc", HGC_adc);
    
    // connected, non connected and calibration
      int selectedChannels[] = {0, 1, 2, 3, 4, 5, 6, 7, 9, 10, 11, 12}; // connected
  //  int selectedChannels[] = {8, 17, 19, 28, 45, 54, 56, 65, 82, 91, 93, 102}; //non connected
  //  int selectedChannels[] = {18, 55, 92, 129, 166, 203, 240, 277, 314, 351, 388, 425}; //Calibration
      const int nChannels = 12;
    
    // Create histograms for each selected channel
    TH1F *histograms[nChannels];
    for(int i = 0; i < nChannels; i++) {
        char histName[50], histTitle[100];
        sprintf(histName, "hist_ch%d", selectedChannels[i]);
        sprintf(histTitle, "HGC ADC Step1 for Channel %d", selectedChannels[i]);
        histograms[i] = new TH1F(histName, histTitle, 60, 60.5, 120.5); 
    }
    
    // Create a Gaussian function for fitting
    TF1 *gauss = new TF1("gauss", "gaus", 60.5, 120.5);
    
    // Fill histograms
    int nEntries = tree->GetEntries();
    for(int i = 0; i < nEntries; i++) {
        tree->GetEntry(i);
        for(int j = 0; j < nChannels; j++) {
            histograms[j]->Fill(HGC_adc[selectedChannels[j]]);
        }
    }
    
    // Create canvas and divide it for multiple plots
    TCanvas *canvas = new TCanvas("canvas", "ADC STEP1 Histograms", 1200, 1200);
    canvas->Divide(3, 4);
    
    // Style settings
    gStyle->SetOptStat(1111);
    gStyle->SetOptFit(1);
    
    // Plot and fit each histogram
    for(int i = 0; i < nChannels; i++) {
        canvas->cd(i + 1);
        
        // Set histogram style
        histograms[i]->SetLineColor(kBlue);
        histograms[i]->SetFillColor(kBlue-10);
        histograms[i]->SetLineWidth(2);
        histograms[i]->GetXaxis()->SetTitle("ADC Counts");
        histograms[i]->GetYaxis()->SetTitle("Entries");
        
        // Fit and draw histogram
        histograms[i]->Fit(gauss, "Q");  // "Q" for quiet mode
        histograms[i]->Draw();

    }
   
    canvas->Write();
    canvas->Print("ADC_step1_histograms.pdf");
    
    // Cleanup
    file->Close();
}


void hgc_adc_adcm1_sub_step2(const char* inputFileName) {
    // Open the root file
    auto inputFile = std::make_unique<TFile>(inputFileName);
    auto tree = dynamic_cast<TTree*>(inputFile->Get("Events"));

    // Variables to store branch data
    Int_t nHGC;
    Float_t HGC_adc[444], HGC_adcm1[444];
    Float_t adc_adcm1_sub[444];

    // Set branch addresses
    tree->SetBranchAddress("nHGC", &nHGC);
    tree->SetBranchAddress("HGC_adcm1", HGC_adcm1);
    tree->SetBranchAddress("HGC_adc", HGC_adc);

    // Define channels to analyze (connected channels)
    const int selectedChannels[] = {0, 1, 2, 3, 4, 5, 6, 7, 9, 10, 11, 12};
    const int nChannels = sizeof(selectedChannels) / sizeof(selectedChannels[0]);

    // Create histograms using smart pointers
    std::vector<std::unique_ptr<TH1F>> histograms;
    for(int i = 0; i < nChannels; i++) {
        auto histName = Form("hist_ch%d", selectedChannels[i]);
        auto histTitle = Form("ADC ADCm Subtracted for Channel %d", selectedChannels[i]);
        histograms.push_back(std::make_unique<TH1F>(histName, histTitle, 60, -30.5, 30.5));
    }

    // Create Gaussian fit function
    auto gauss = std::make_unique<TF1>("gauss", "gaus", -30.5, 30.5);

    // Fill histograms
    int nEntries = tree->GetEntries();
    
    for(int i = 0; i < nEntries; i++) {
        tree->GetEntry(i);
        for(int j = 0; j < nChannels; j++) {
            adc_adcm1_sub[j] = HGC_adc[j] - HGC_adcm1[j];  // Calculate the subtracted ADC values
            histograms[j]->Fill(adc_adcm1_sub[j]);  // Fill histograms 
        }
    }

    // Create canvas
    auto canvas = std::make_unique<TCanvas>("canvas", "ADC ADCm Subtracted Distributions", 1200, 1200);
    canvas->Divide(3, 4);

    // Style settings
    gStyle->SetOptStat(1111);
    gStyle->SetOptFit(1);

    // Plot and fit histograms
    for(int i = 0; i < nChannels; i++) {
        canvas->cd(i + 1);

        // Set histogram style
        histograms[i]->SetLineColor(kBlue);
        histograms[i]->SetFillColor(kBlue-10);
        histograms[i]->SetLineWidth(2);
        histograms[i]->GetXaxis()->SetTitle("ADC Counts");
        histograms[i]->GetYaxis()->SetTitle("Entries");

        // Fit and draw
        histograms[i]->Fit(gauss.get(), "Q");
        histograms[i]->Draw();
    }

    // Save output
    canvas->Print("HGC_ADC_ADCm_Subtracted_Histograms_Step2.pdf");
}

void plot_adc_adcm1_subtracted_step3() {
    // Open the file containing CM subtracted data
    TFile *file = new TFile("HGC_adc_adcm1_sub.root");
    TTree *tree = (TTree*)file->Get("Events");

    // Variables to store data
    Int_t nHGC;
    Float_t adc_adcm1_sub[444];

    // Set branch addresses
    tree->SetBranchAddress("nHGC", &nHGC);
    tree->SetBranchAddress("adc_adcm1_sub", adc_adcm1_sub);

      int selectedChannels[] = {0, 1, 2, 3, 4, 5, 6, 7, 9, 10, 11, 12}; // connected
  //  int selectedChannels[] = {8, 17, 19, 28, 45, 54, 56, 65, 82, 91, 93, 102}; //non connected
  //  int selectedChannels[] = {18, 55, 92, 129, 166, 203, 240, 277, 314, 351, 388, 425}; //Calibration
      const int nChannels = 12;

    // Create histograms for each selected channel
    TH1F *histograms[nChannels];
    for(int i = 0; i < nChannels; i++) {
        char histName[50], histTitle[100];
        sprintf(histName, "hist_ch%d", selectedChannels[i]);
        sprintf(histTitle, "ADC ADCm Subtracted for Channel %d", selectedChannels[i]);
        histograms[i] = new TH1F(histName, histTitle, 60, -30.5, 30.5);  
    }

    // Create a Gaussian function for fitting
    TF1 *gauss = new TF1("gauss", "gaus", -30.5, 30.5);

    // Fill histograms
    int nEntries = tree->GetEntries();
    for(int i = 0; i < nEntries; i++) {
        tree->GetEntry(i);
        for(int j = 0; j < nChannels; j++) {
            histograms[j]->Fill(adc_adcm1_sub[selectedChannels[j]]);
        }
    }

    // Create canvas and divide it for multiple plots
    TCanvas *canvas = new TCanvas("canvas", "adcm1 Subtracted Distributions", 1200, 1200);
    canvas->Divide(3, 4);

    // Style settings
    gStyle->SetOptStat(1111);
    gStyle->SetOptFit(1);

    // Plot and fit each histogram
    for(int i = 0; i < nChannels; i++) {
        canvas->cd(i + 1);

        // Set histogram style
        histograms[i]->SetLineColor(kBlue);
        histograms[i]->SetFillColor(kBlue-10);
        histograms[i]->SetLineWidth(2);
        histograms[i]->GetXaxis()->SetTitle("ADC Counts");
        histograms[i]->GetYaxis()->SetTitle("Entries");

        // Fit and draw histogram
        histograms[i]->Fit(gauss, "Q");  // "Q" for quiet mode
        histograms[i]->Draw();


    }
    // Save canvas to output file
    canvas->Write();

    // Save as PDF for easy viewing
    canvas->Print("HGC_ADC_ADCm_Sutracted_Step3.pdf");

    // Cleanup
    file->Close();
}

void plot_cm_step1() {
    TFile *file = new TFile("ADC_CM_extracted_branches.root");
    TTree *tree = (TTree*)file->Get("Events");

    const int N_HGC = 444;
    const int N_HGC_CM = 24;
    
    Int_t nHGC;
 //   Float_t         HGC_adc[N_HGC];
 //   Float_t         HGC_adcm1[N_HGC];    //remove comments for other steps when u need adc adcm
    Float_t         HGCCM_cm[N_HGC_CM];

    tree->SetBranchAddress("nHGC", &nHGC);
    tree->SetBranchAddress("HGCCM_cm", HGCCM_cm);

    
    // connected, non connected and calibration is not in CM
      int selectedChannels[] = {0, 1, 2, 3, 4, 5, 6, 7, 9, 10, 11, 12,}; // connected
  //  int selectedChannels[] = {8, 17, 19, 28, 45, 54, 56, 65, 82, 91, 93, 102}; //non connected
  //  int selectedChannels[] = {18, 55, 92, 129, 166, 203, 240, 277, 314, 351, 388, 425}; //Calibration
    const int nChannels = 12;
    
    // Create histograms for each selected channel
    TH1F *histograms[nChannels];
    for(int i = 0; i < nChannels; i++) {
        char histName[50], histTitle[100];
        sprintf(histName, "hist_ch%d", selectedChannels[i]);
        sprintf(histTitle, "HGCCM Step1 for Channel %d", selectedChannels[i]);
        histograms[i] = new TH1F(histName, histTitle, 60, 60.5, 120.5); 
    }
    
    // Create a Gaussian function for fitting
    TF1 *gauss = new TF1("gauss", "gaus", 60.5, 120.5);
    
    // Fill histograms
    int nEntries = tree->GetEntries();
    for(int i = 0; i < nEntries; i++) {
        tree->GetEntry(i);
        for(int j = 0; j < nChannels; j++) {
            histograms[j]->Fill(HGCCM_cm[selectedChannels[j]]);
        }
    }
    
    // Create canvas and divide it for multiple plots
    TCanvas *canvas = new TCanvas("canvas", "CM STEP1 Histograms", 1200, 1200);
    canvas->Divide(3, 4);
    
    // Style settings
    gStyle->SetOptStat(1111);
 //   gStyle->SetOptFit(1);
    
    // Plot and fit each histogram
    for(int i = 0; i < nChannels; i++) {
        canvas->cd(i + 1);
        
        // Set histogram style
        histograms[i]->SetLineColor(kBlue);
        histograms[i]->SetFillColor(kBlue-10);
        histograms[i]->SetLineWidth(2);
        histograms[i]->GetXaxis()->SetTitle("ADC Counts");
        histograms[i]->GetYaxis()->SetTitle("Entries");
        
        // Fit and draw histogram
  //      histograms[i]->Fit(gauss, "Q");  // "Q" for quiet mode
        histograms[i]->Draw();

    }
   
    canvas->Write();
  
    canvas->Print("cm_step1_histograms_no_fit.pdf");
    
    // Cleanup
    file->Close();
}

void plot_cm_step2() {
    const int N_HGC_CM = 24;

    // Open input file using unique_ptr (open in read mode)
    auto file = std::make_unique<TFile>("hgcCM_meansubtracted.root", "READ");
    auto tree = dynamic_cast<TTree*>(file->Get("Events"));

    // Input variables
    Float_t HGCCM_CMMEANsubtracted[N_HGC_CM];

    // Set branch address
    tree->SetBranchAddress("HGCCM_CMMEANsubtracted", HGCCM_CMMEANsubtracted);

    // Connected channels
    std::vector<int> selectedChannels = {0, 1, 2, 3, 4, 5, 6, 7, 9, 10, 11, 12};
    const int nChannels = selectedChannels.size();

    // Create histograms for selected channels
    std::vector<std::unique_ptr<TH1F>> histograms;
    for(int ch : selectedChannels) {
        auto histName = Form("hist_ch%d", ch);
        auto histTitle = Form("HGCCM Step2 for Channel %d", ch);
        histograms.push_back(std::make_unique<TH1F>(histName, histTitle, 61, -30.5, 30.5));
    }

    // Create a Gaussian function for fitting
    auto gauss = std::make_unique<TF1>("gauss", "gaus", -30.5, 30.5);

    // Fill histograms
    int nEntries = tree->GetEntries();
    for(int i = 0; i < nEntries; i++) {
        tree->GetEntry(i);
        for(size_t j = 0; j < selectedChannels.size(); j++) {
            histograms[j]->Fill(HGCCM_CMMEANsubtracted[selectedChannels[j]]);
        }
    }

    // Create canvas 
    auto canvas = std::make_unique<TCanvas>("canvas", "CM STEP2 Histograms", 1200, 1200);
    canvas->Divide(3, 4);

    // Style settings
    gStyle->SetOptStat(1111);
   gStyle->SetOptFit(1);

    // Plot and fit each histogram
    for(size_t i = 0; i < nChannels; i++) {
        canvas->cd(i + 1);

        // Set histogram style
        histograms[i]->SetLineColor(kBlue);
        histograms[i]->SetFillColor(kBlue-10);
        histograms[i]->SetLineWidth(2);
        histograms[i]->GetXaxis()->SetTitle("ADC Counts");
        histograms[i]->GetYaxis()->SetTitle("Entries");

        // Fit and draw histogram
        histograms[i]->Fit(gauss.get(), "Q");  // "Q" for quiet mode
        histograms[i]->Draw();
    }

    // Save PDF
    canvas->Print("cm_step2_histograms.pdf");

    // No need to write to the input file, as it's opened in read mode
}


void calculate_cm_mean_subtracted_avg() {
    // Constants
    const int N_HGC_CM = 24;  // Total number of channels
    const int N_ROC_HALVES = N_HGC_CM / 2;  // Number of ROC halves

    auto inputFile = std::make_unique<TFile>("tryhgcCM_meansubtracted.root", "READ");

    auto inputTree = dynamic_cast<TTree*>(inputFile->Get("Events"));
 
    auto outputFile = std::make_unique<TFile>("CMMEANsubtracted_ROC_AVG_TWO_CHANNELS_step3.root", "RECREATE");
    auto outputTree = std::make_unique<TTree>("Events", "CM Mean Subtracted Averaged Channels");

    // Input variables
    Float_t HGCCM_CMMEANsubtracted[N_HGC_CM];
    
    // Output variables
    Float_t HGCCM_CMMEANsubtracted_avgflt[N_ROC_HALVES];

    // Set input branch address
    inputTree->SetBranchAddress("HGCCM_CMMEANsubtracted", HGCCM_CMMEANsubtracted);

    // Create output branch
    outputTree->Branch("HGCCM_CMMEANsubtracted_avgflt", HGCCM_CMMEANsubtracted_avgflt, 
                      "HGCCM_CMMEANsubtracted_avgflt[12]/F");

    // Create histograms for visualization
    std::vector<std::unique_ptr<TH1F>> histograms;
    std::vector<std::unique_ptr<TF1>> gaussFits;
    
    // Initialize histograms (one per ROC half)
    for (int ch = 0; ch < N_ROC_HALVES; ++ch) {
        auto histName = Form("hist_avg_ROC%d", ch);
        auto histTitle = Form("CM Mean Subtracted Avg for ROC Half %d (Ch %d & %d)", ch, ch * 2, ch * 2 + 1);
        histograms.push_back(std::make_unique<TH1F>(histName, histTitle, 41, -20.5, 20.5));
        gaussFits.push_back(std::make_unique<TF1>(Form("gauss_ROC%d", ch), "gaus", -20.5, 2.5));
    }

    // Process entries
    int nEntries = inputTree->GetEntries();
    std::cout << "Processing " << nEntries << " entries..." << std::endl;
    
    for (int i = 0; i < nEntries; ++i) {
        inputTree->GetEntry(i);

        // Calculate average for each pair of channels (per half ROC)
        for (int j = 0; j < N_ROC_HALVES; ++j) {
            // Calculate average of two consecutive channels
            HGCCM_CMMEANsubtracted_avgflt[j] = (HGCCM_CMMEANsubtracted[j * 2] + HGCCM_CMMEANsubtracted[j * 2 + 1]) / 2.0;

            // Fill histogram for this ROC half
            histograms[j]->Fill(HGCCM_CMMEANsubtracted_avgflt[j]);
        }

        // Fill output tree
        outputTree->Fill();
        

    }

    // Create canvas for visualization
    auto canvas = std::make_unique<TCanvas>("canvas", "CM Mean Subtracted Averaged Channels", 1600, 1200);
    canvas->Divide(3, 4);

    // Plot histograms with Gaussian fits
    gStyle->SetOptStat(1111);
    gStyle->SetOptFit(1);
    
    for (size_t h = 0; h < histograms.size(); ++h) {
        canvas->cd(h + 1);
        
        // Fit Gaussian
        histograms[h]->Fit(gaussFits[h].get(), "Q");
        
        // Set histogram style
        histograms[h]->SetLineColor(kBlue);
        histograms[h]->SetFillColor(kBlue - 10);
        histograms[h]->SetLineWidth(2);
        histograms[h]->GetXaxis()->SetTitle("Averaged ADC Counts");
        histograms[h]->GetYaxis()->SetTitle("Entries");
        histograms[h]->Draw();

        // Get fit parameters
        double mean = gaussFits[h]->GetParameter(1);
        double sigma = gaussFits[h]->GetParameter(2);
        double chi2ndf = gaussFits[h]->GetChisquare() / gaussFits[h]->GetNDF();

        // Print fit results
        std::cout << "ROC Half " << h << ": Mean = " << mean 
                  << ", Sigma = " << sigma 
                  << ", Chi2/NDF = " << chi2ndf << std::endl;
    }

    // Save output
    outputFile->cd();
    outputTree->Write();
    canvas->Write();
    canvas->Print("cm_mean_subtracted_avg_distributions.pdf");

    std::cout << "Processing complete. Output saved to CMMEANsubtracted_ROC_AVG_TWO_CHANNELS_step3.root" << std::endl;
}

void subtract_cm(const char* cmFileName, const char* adcFileName, const char* outputFileName) {
    // Open the input root files
    auto cmFile = std::make_unique<TFile>(cmFileName);
    auto adcFile = std::make_unique<TFile>(adcFileName);

    // Get the trees from both files
    TTree *cmTree = (TTree*)cmFile->Get("Events");
    TTree *adcTree = (TTree*)adcFile->Get("Events");

    // Create output file
    auto outputFile = std::make_unique<TFile>(outputFileName, "RECREATE");

    // Variables to store branch data
    Float_t HGCCM_CMMEANsubtracted_avgflt[24];  // CM values for 24 channels (2 per half ROC)
    Float_t adc_adcm1_sub[444];                 // ADC values for 444 channels
    Float_t final_subtracted[444];              // Array to store final subtracted values
    Int_t nHGC;

    // Set branch addresses
    cmTree->SetBranchAddress("HGCCM_CMMEANsubtracted_avgflt", HGCCM_CMMEANsubtracted_avgflt);
    adcTree->SetBranchAddress("adc_adcm1_sub", adc_adcm1_sub);
    adcTree->SetBranchAddress("nHGC", &nHGC);

    // Create output tree and branch
    TTree *outputTree = new TTree("Events", "Events");
    outputTree->Branch("nHGC", &nHGC, "nHGC/I");
    outputTree->Branch("final_subtracted", final_subtracted, "final_subtracted[444]/F");

    // Get number of entries
    Long64_t nentries = std::min(cmTree->GetEntries(), adcTree->GetEntries());

    // Process entries
    for (Long64_t entry = 0; entry < nentries; ++entry) {
        cmTree->GetEntry(entry);
        adcTree->GetEntry(entry);

        // Process each half ROC
        for (int halfROC = 0; halfROC < 12; ++halfROC) {
            int cmIndex = halfROC * 2;  // Index for CM values (2 per half ROC)
            int startChannel = halfROC * 37;  // Starting channel for this half ROC

            // Get the CM value for this half ROC
            float cmValue = HGCCM_CMMEANsubtracted_avgflt[cmIndex];  // Using first of the two CM values

            // Subtract CM from all channels in this half ROC
            for (int chan = 0; chan < 37; ++chan) {
                int globalChan = startChannel + chan;
                final_subtracted[globalChan] = adc_adcm1_sub[globalChan] - cmValue;
            }
        }

        outputTree->Fill();
    }

    // Write and close
    outputTree->Write();
    outputFile->Close();
    cmFile->Close();
    adcFile->Close();
}

int main() {
    plot_ADC_step1();
    hgc_adc_adcm1_sub_step2("ADC_CM_extracted_branches.root");
    plot_adc_adcm1_subtracted_step3();
    plot_cm_step1();
    plot_cm_step2();
    calculate_cm_mean_subtracted_avg();
    subtract_cm("CMMEANsubtracted_ROC_AVG_TWO_CHANNELS_step3.root",
                "HGC_adc_adcm1_sub.root",
                "final_subtraction_adc_cm.root");
    return 0;
}


