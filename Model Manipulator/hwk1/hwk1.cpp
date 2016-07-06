// hwk1.cpp : Defines the entry point for the console application.

#include <fstream>
#include <iostream>
#include <string>
#include <cstring>
using namespace std;

#include "stdafx.h"

int main(int argc, char* argv[]){

    if(argc!=2)
    {
        cout << "Invalid number/type of arguments.\nEnter complete file path to the s3d file as the single function argument.\n" << endl;
        return 0;
    }

    string oldFileName = argv[1];
    string oldFileNameSave = oldFileName;
    
    bool fileOpen = 1;

    ifstream inputFile(oldFileName);

	if ( !inputFile.is_open ())
	{
		fileOpen = 0;
	}

    if(fileOpen==0)
    {
        cout << "\n\n\nThe s3d file could not be found.\nEnter complete file path to the s3d file as the single function argument.\n" << endl;
        return 0;
    }
    
    cout << "\n\n\nThe file to be altered is:\n\n" << oldFileName <<"\n\nHow do you want to alter it?" << endl;
    
    int option = 7;

    while((option<1)||(option>6))
    {
        cout << "\n0 - Exit program and do nothing.\n";
        cout << "1 - Do not change anything, but copy to a new file.\n";
        cout << "2 - Scale the vertices.\n";
        cout << "3 - Set the origin at the center of the model\n";
        cout << "4 - Set the origin at the bottom center of the model\n";
        cout << "5 - Rotate 90 degrees clockwise\n";
        cout << "6 - Tanslate\n";
        cout << "\nEnter a number to select an option...\n";
        
        cin >> option;

        if(option==0)
        {
            cout << "\nExiting program.\n";
            return 0;
        }
        else if((option<1)||(option>6))
        {
            cout << "\nInvalid input.\n";
        }
    }

    float xscalar = 1;
    float yscalar = 1;
    float zscalar = 1;
    float scalar = 1;
    float axis = 0;

    if(option==2)
    {
        cout << "\nX:Enter a value between -999.999 and 999.999  by which to scale the vertices...\n";

        cin >> xscalar;

        cout << "\nY:Enter a value between -999.999 and 999.999  by which to scale the vertices...\n";

        cin >> yscalar;

        cout << "\nZ:Enter a value between -999.999 and 999.999  by which to scale the vertices...\n";

        cin >> zscalar;
        if((xscalar>999.999)||(xscalar<-999.999))
        {
            cout << "\nValue out of range, exiting program.\n";
            return 0;
        }
        if((yscalar>999.999)||(yscalar<-999.999))
        {
            cout << "\nValue out of range, exiting program.\n";
            return 0;
        }
        if((zscalar>999.999)||(zscalar<-999.999))
        {
            cout << "\nValue out of range, exiting program.\n";
            return 0;
        }
    }

    if(option==6)
    {
        cout << "\nAxis? (1 for x, 2 for y, 3 for z)\n";
        cin >> axis;

        if((axis<1)||(axis>3))
        {
            cout << "\nValue out of range, exiting program.\n";
            return 0;
        }

        cout << "\nEnter a value between -999.999 and 999.999  by which to translate the vertices...\n";

        cin >> scalar;

        if((scalar>999.999)||(scalar<-999.999))
        {
            cout << "\nValue out of range, exiting program.\n";
            return 0;
        }
    }


    int loc = oldFileName.length() - 4;

    oldFileName[loc] = 'n';
    oldFileName[loc+1] = 'e';
    oldFileName[loc+2] = 'w';
    oldFileName[loc+3] = '.';
    
    string newFileName = oldFileName + "s3d";
    
    //system("Pause");

    ofstream outputFile;
    outputFile.open(newFileName);

    float vertx;
    float verty;
    float vertz;
    
    float vertCounter = 0;

    float vertxTotal = 0;
    float vertyTotal = 0;
    float vertzTotal = 0;

    float vertxAvg = 0;
    float vertyAvg = 0;
    float vertzAvg = 0;

    float vertzMin = 0;

  bool bSeeingVertices = false; //are we seeing vertices?
  string buffer; //to hold a line of text
  char* p; //pointer to start of buffer
  while(getline(inputFile, buffer, '\n')){ //get a line of the input
    p = (char*)buffer.c_str(); //get pointer to start of buffer
    if(bSeeingVertices){ //seeing vertices
      bSeeingVertices = strstr(p, "//") == 0; //check for end of vertices
      if(bSeeingVertices)
      {
          sscanf(p,"%f,%f,%f",&vertx,&verty,&vertz);
          if(option==2)
          {
            vertx = vertx*xscalar;
            verty = verty*yscalar;
            vertz = vertz*zscalar;
          }
          if(option==3)
          {
              vertxTotal = vertxTotal + vertx;
              vertyTotal = vertyTotal + verty;
              vertzTotal = vertzTotal + vertz;
          }
          if(option==4)
          {
              vertxTotal = vertxTotal + vertx;
              vertyTotal = vertyTotal + verty;
              vertzTotal = vertzTotal + vertz;
              if(vertCounter==0)
              {
                  vertzMin = vertz;
              }
              else
              {
                  if(vertz<vertzMin)
                      vertzMin = vertz;
              }
          }
          if(option==5)
          {
              float vertxTemp = vertx;
              vertx = verty;
              verty = vertxTemp*(-1.0f);
          }
          if(option==6)
          {
              if(axis==1)
                  vertx = vertx + scalar;
              else if(axis==2)
                  verty = verty + scalar;
              else if(axis==3)
                  vertz = vertz + scalar;
          }

          vertCounter = vertCounter + 1;

          if((option!=3)&&(option!=4))
              outputFile << vertx << "," << verty << "," << vertz << endl;
      }
      else //not a vertex, so just output the line
          if((option!=3)&&(option!=4))
              outputFile << buffer << endl;
    }
    else{ //not seeing vertices, output the line and look for start of vertices marker
      bSeeingVertices = strstr(p, "// vertList:") != 0;
      if((option!=3)&&(option!=4))
          outputFile << buffer << endl;
    }
  } //while


  if((option==3)||(option==4))
  {
      vertxAvg = vertxTotal/vertCounter;
      vertyAvg = vertyTotal/vertCounter;
      vertzAvg = vertzTotal/vertCounter;

      inputFile.close();
      inputFile.open(oldFileNameSave);

      bSeeingVertices = false; //are we seeing vertices?

        while(getline(inputFile, buffer, '\n')){ //get a line of the input
        p = (char*)buffer.c_str(); //get pointer to start of buffer
        if(bSeeingVertices){ //seeing vertices
          bSeeingVertices = strstr(p, "//") == 0; //check for end of vertices
          if(bSeeingVertices)
          {
              sscanf(p,"%f,%f,%f",&vertx,&verty,&vertz);

              if(option==3)
              {
                  vertx = vertx - vertxAvg;
                  verty = verty - vertyAvg;
                  vertz = vertz - vertzAvg;
              }
              if(option==4)
              {
                  vertx = vertx - vertxAvg;
                  verty = verty - vertyAvg;
                  vertz = vertz - vertzMin;
              }
              outputFile << vertx << "," << verty << "," << vertz << endl;
          }
          else //not a vertex, so just output the line
            outputFile << buffer << endl;
        }
        else{ //not seeing vertices, output the line and look for start of vertices marker
          bSeeingVertices = strstr(p, "// vertList:") != 0;
          outputFile << buffer << endl;
        }
      } //while
  } //if

  cout << "\n\nThe operation is complete and the new file is:\n\n" << newFileName << endl << endl;

  inputFile.close();
  outputFile.close();
  return 0;
}

