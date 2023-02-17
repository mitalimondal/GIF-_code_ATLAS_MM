//**************************************************************************************
//                                                                                     *
//           This code is written by DEBABRATA BHOWMIK & MITALI MONDAL                 *
//                                                                                     *
//**************************************************************************************

#include <iostream>
#include <fstream>
#include <vector>
#include "TGraph.h"
#include "TVector.h"
using namespace std;

double yvalAtx(double p0, double p1, double p2, double vForMin){
  double yval = p0+p1*vForMin+p2*pow(vForMin,2);
  return yval;
}

double distVec(vector<double> vec){
  double dd=0;
  //for(int i=0;i<vec.size(); i++){
  //dd += abs(vec[i+1]-vec[i]);
  for(int i=0;i<vec.size(); i++){
    for(int j = i+1; j < vec.size(); j++){
      dd += abs(vec[i]-vec[j]);
    }
  }
  return dd;
}

void I_scaled_mod() {

  gStyle->SetOptFit(0); 
  // Specify the common part of the file names
  string filename_base = "att";
  
  // Read the data from the nine text files
  vector<vector<vector<double> > >data3D;
  double IScaling[9]={1, 1.2, 2.2, 4.5, 8.8, 16.279, 34.23, 73.5, 142.3};
  for (int i = 0; i < 9; i++){
    string filename = filename_base + to_string(i+1) + ".txt";
    ifstream file(filename);
    if (!file.is_open()) {
      cout << "Error: Unable to open file " << filename << endl;
    }
    cout<< "Opened File " << filename <<endl; 
    vector<double> voltage, current, current_scale;
    double V, I;
    while (file >> V >> I) {
      //cout<< V << "\t"<< I <<endl;
      voltage.push_back(V);
      current.push_back(I);
      current_scale.push_back(I*IScaling[i]/73.5);      
    }
    file.close();
    cout<<"Successfully closed file "<<filename<<endl;
    data3D.push_back(vector<vector<double>>());
    data3D[i].push_back(voltage);
    data3D[i].push_back(current);
    data3D[i].push_back(current_scale);
  }// Open the file for reading
  
  double distMin=99999.0;
  double R_min=-9999;
  
  double targetVoltage;
  cout<<"Please give me the value of the voltage for which you want to minimize the distance between graphs"<<endl;
  cin>>targetVoltage;

  vector<double> vcorr;
  vector<double> datavoltages;
  char cNamepng[50],cNamepdf[50],cNameroot[50],grTitle[50];

  TF1 *fitFunc = new TF1("fitFunc", "pol2");
  TGraph* gr2 = new TGraph();
  for(double R=0.0; R<2.0; R+=0.01){
    TMultiGraph *mg = new TMultiGraph();
    vector<double> IcorrAtx;
    auto legend = new TLegend(0.18,0.3,0.38,0.89);
    legend->SetBorderSize(0);
    for(int i=0; i<data3D.size();i++){
      vcorr.clear();
      datavoltages.clear();
      datavoltages.assign(data3D[i][0].begin(), data3D[i][0].end());
      //std::copy(data3D[i][0].begin(), datavoltages.end(), back_inserter(datavoltages)); 
      std::transform(data3D[i][0].begin(), data3D[i][0].end(), data3D[i][1].begin(), std::back_inserter(vcorr),[&R](double a, double b) { return a - b*R; });
      //cout<<"vcorr size = "<<vcorr.size()<<endl;
      //cout<<"data3D[i][2] size = "<<data3D[i][2].size()<<endl;

      //cout<<data3D[i][1][0]<<"\t"<<data3D[i][1][1]<<"\t"<<data3D[i][1][2]<<endl;
      TGraph* gr = new TGraph(vcorr.size(),&vcorr[0],&data3D[i][2][0]);
      gr->SetMarkerColor(i+1);
      gr->SetMarkerSize(1.5);
      gr->SetMarkerStyle(20);
      gr->SetLineColor(i+1);
      gr->SetLineWidth(3);
      if(R==0.0){
	gr->Fit(fitFunc, "0Q");
      }
      else{
	gr->Fit(fitFunc, "0Q");
      }
      //if(R==0 && i==0) gr->Draw();
      mg->Add(gr);
      legend->AddEntry(gr,Form("Att=%0.1f",IScaling[i]),"lp");
            
      double p0 = fitFunc->GetParameter(0);
      double p1 = fitFunc->GetParameter(1);
      double p2 = fitFunc->GetParameter(2);

      /*
      TFitResultPtr r = gr->Fit("fitFunc","S");

      double p0 = r->Parameter(0);
      double p1 = r->Parameter(1);
      double p2 = r->Parameter(2);
      */
      
      double y_current=yvalAtx(p0, p1, p2, targetVoltage);
      IcorrAtx.push_back(y_current);
      //std::vector<double> y_current;
      //for(int k =0; k<datavoltages.size(); k++){
      //y_current[k]=yvalAtx(p0,p1,p2,datavoltages[k]);
      //cout<<"y_current\t"<<y_current[k]<<endl;
      //cout<<data3D[i][0][k]<<"\t"<<vcorr[k]<<"\t"<<data3D[i][2][k]<<"\t"<<datavoltages[k]<<endl;    
      //}
    } //End loop over files
    
    double dist=distVec(IcorrAtx);
    if(dist<distMin){
      distMin=dist;
      R_min=R;
      cout<<"distance value\t"<<dist<<"\tR\t"<<R<<endl;
    }
    
    TCanvas *c1 = new TCanvas("c1","A Simple Graph Example",1100,900);
    //c1->SetGrid();
    c1->SetLeftMargin(0.15);
    c1->SetBottomMargin(0.15);
    c1->SetFrameLineWidth(3);
    sprintf(grTitle,"For R=%.2f",R);
    mg->SetTitle(grTitle);
    mg->Draw("APL");
    //mg->GetXaxis()->SetRangeUser(400,540);
    
    mg->GetXaxis()->CenterTitle(true);
    mg->GetXaxis()->SetTitleFont(22);
    mg->GetXaxis()->SetLabelFont(22);
    mg->GetXaxis()->SetTitleOffset(1.3);
    mg->GetXaxis()->SetTitle("HV_{corrected}(V)");
    mg->GetYaxis()->CenterTitle(true);
    mg->GetYaxis()->SetTitleFont(22);
    mg->GetYaxis()->SetLabelFont(22);
    mg->GetYaxis()->SetTitleOffset(1.7);
    mg->GetYaxis()->SetTitle("I_{scaled}(#muA)");
    if(R==0.0)mg->GetXaxis()->SetTitle("HV(V)");
    //mg->Draw("APL");
    c1->Update();
    legend->Draw();
    c1->Modified();
    //sprintf(cNamepng,"IVFit_R_%.2f.png",R);
    sprintf(cNameroot,"IVFit_R_%0.2f.root",R);
    //sprintf(cNamepdf,"IVFit_R_%.2f.pdf",R);
    //c1->SaveAs(cNamepng);
    c1->SaveAs(cNameroot);
    //c1->SaveAs(cNamepdf);
    delete c1;
    gr2->AddPoint(R, dist);
  } //End iteration over R

  cout<< "Ok I found you will get the maximally squzeed graphs at R = "<<R_min<< " with the sum of distance between the graphs to be "<< distMin<<endl;

  gr2->SetTitle("R vs distance between curves with different att");
  gr2->SetMarkerColor(kRed);
  gr2->SetMarkerSize(1.5);
  gr2->SetMarkerStyle(20);
  gr2->SetLineColor(kBlue);
  gr2->SetLineWidth(3);
  gr2->GetXaxis()->SetTitle("R (M#Omega");
  gr2->GetYaxis()->SetTitle("D");

  TCanvas *c2 = new TCanvas("c2","A Simple Graph Example",700,900);
  c2->cd();
  gr2->Draw("APL");
  c2->SaveAs("Rvsdist.png");
  c2->SaveAs("Rvsdist1.root");
}
