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


int main() {
    plot_ADC_step1();
    hgc_adc_adcm1_sub_step2("ADC_CM_extracted_branches.root");
    plot_adc_adcm1_subtracted_step3();
    plot_cm_step1();
 
    return 0;
}
