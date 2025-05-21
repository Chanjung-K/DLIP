############################################################################
# @course 	Deep Learning and Image Processing - HGU
# @author	Chanjung Kim / 22000188
# @Created	2025-04-22 by CJKIM
# @Modified	2025-05-07 by CJKIM
# @brief	[DLIP] LAB for Tension Detection of Rolling Metal Sheet
############################################################################

import cv2 as cv
import numpy as np
import matplotlib as plt
from matplotlib import pyplot as ppt

# Video load
deo = cv.VideoCapture("LAB3_Video.mp4"); 
open, src = deo.read(); 

if not open:
    print("Error: no video is opened"); 
    deo.release(); 
    exit(); 

rows, cols, _ = src.shape; 
Output = cv.VideoWriter("Output#1.mp4", cv.CAP_PROP_FOURCC, 30.0, (cols, rows), True); 

if not Output.isOpened():
    print("Error: No writer is opened"); 
    Output.release(); 
    exit(); 
######################################## Parameter Definition ########################################
# rows, cols calcu
Lv2 = 250; Lv3 = 120; 
dst = np.zeros(src.shape, np.uint8); 
dst = cv.cvtColor(dst, cv.COLOR_RGB2GRAY); 

# roi setting
width = int(cols * 0.45); 
HStart= int(rows*0.37); 
HLimit = rows; Level = 0; 
alpha = 1.6; beta = [600, 1976]; 

# Inrange Parameter
lower_b = np.array([0, 0, 150]); 
upper_b = np.array([255, 230, 255]); 

# Morphology Kernel Size
k_3 = cv.getStructuringElement(cv.MORPH_RECT, (3,3)); k_5 = cv.getStructuringElement(cv.MORPH_RECT, (5,5)); 
k_6 = cv.getStructuringElement(cv.MORPH_RECT, (6,6)); k_7 = cv.getStructuringElement(cv.MORPH_RECT, (7,7)); 
k_13 = cv.getStructuringElement(cv.MORPH_RECT, (13,13)); 
RectKennelSize = 5; 
k_51 = cv.getStructuringElement(cv.MORPH_RECT, (5,1)); k_15 = cv.getStructuringElement(cv.MORPH_RECT, (1,RectKennelSize)); 
FirstInterval = True; 
count = 0; 

############################################### VIDEO ###############################################
while deo.isOpened():
    state, src = deo.read(); 
    if not state:
        break; 
    dst = np.zeros(src.shape, np.uint8); 

    # ROI Selection and Splitting
    roi = src[0:rows, 0:width]; 
    b, g, r = cv.split(roi); 

    # RGB --> HSV Conversion
    temp = cv.cvtColor(roi, cv.COLOR_RGB2HSV); 

    # Inrange White Color
    Inrange = cv.inRange(temp, lower_b, upper_b); 

    # Substraction r-Inrange is not permitted since most of intensity goes negative
    r_temp = np.zeros(r.shape, np.uint8); 
    for i in range(rows):
        for j in range(width): 
            if not (Inrange[i][j]):
                r_temp[i][j] = r[i][j]; 

    ############################ CANNY #############################
    canr = cv.Canny(r_temp, 50, 150); 

    ########################## morphology ##########################
    Inrange = cv.morphologyEx(Inrange, cv.MORPH_DILATE, k_3); 
    canr = cv.morphologyEx(canr, cv.MORPH_CLOSE, k_3); 
    r = np.subtract(canr, Inrange); 
    r = cv.morphologyEx(r, cv.MORPH_DILATE, k_51); 
    r = cv.morphologyEx(r, cv.MORPH_OPEN, k_15);    
    r = cv.morphologyEx(r, cv.MORPH_CLOSE, k_13); 
    r = cv.morphologyEx(r, cv.MORPH_OPEN, k_51); 
    _, r = cv.threshold(r, 100, 255, cv.THRESH_BINARY); 
    
    ########################### Contours ###########################
    contours,_ = cv.findContours(r, cv.RETR_LIST, cv.CHAIN_APPROX_SIMPLE); 
    for i in range (0, len(contours)):
        if cv.contourArea(contours[i]) < 300:
            continue; 
        cv.drawContours(dst, contours, i, (255,255,255), -1); 
    
    ######################## Eliminate Noise ########################
    for j in range(600, rows):
        xidx1 = int((j-beta[0])/alpha); 
        for i in range(xidx1):
            dst[j][i] = 0; dst[j][width - i] = 0; 

    ############################ PolyFit ############################
    y_sample, x_sample,_ = np.nonzero(dst[HStart:HLimit][:]); 
    y_sample += HStart; 
    model_poly = np.polyfit(x_sample, y_sample, 2); 

    if FirstInterval:
        model_prev = model_poly; 
        FirstInterval = False; 
    
    ####################### PolyFit Exception ########################
    if model_poly[0] > -0.0028 or model_poly[0] < -0.00357 or model_poly[2] < 250 or (model_poly[2] > 400 + 50* (Level-1)):        
        model_poly = model_prev; 
    else:
        model_prev = model_poly; 

    ########################## Point assign ##########################
    x = np.linspace(int(width*0.05), width, width - int(width*0.05)); 
    y = np.polyval(model_poly, x); 
    y = np.array(y[y > y[0]]).astype(np.int32); 
    x = np.array(x[0:y.size]).astype(np.int32); 

    ########################## Draw in src ###########################
    for i in range(len(x)):
        for j in range(-2, 2, 1):
            yidx = y[i]+j; xidx = x[i]+j; 
            # index exception
            if yidx > rows or yidx < 0 or xidx < 0 or xidx >= width: 
                continue; 
            src[yidx][xidx] = [0, 255, 0];  

    
    # ymax exception in a condition y is empty
    if y.size == 0:
        y_max = 0;   
    else:
        y_max = rows - np.max(y); 
    
    ##################### Figure out Score and Level  #####################
    if y_max < Lv3:
        HLimit = rows; 
        Level = 3; 
    elif (y_max < Lv2):
        HLimit = rows - Lv3; 
        Level = 2; 
    else:
        HLimit = rows - Lv2; 
        Level = 1; 
    
    ########################## Text Insertion  ##########################
    StrScore = "Score: " + str(y_max); 
    StrLevel = "Level: " + str(Level); 
    src = cv.putText(src, StrScore, (width+100, 200), cv.FONT_HERSHEY_PLAIN, 2.0, (255,255,255), 4); 
    src = cv.putText(src, StrLevel, (width+100, 240), cv.FONT_HERSHEY_PLAIN, 2.0, (255,255,255), 4); 

    # Output print
    cv.imshow("src", src); 

    # Uncomment to write the result
    # Output.write(src); 

    if cv.waitKey(30) == 27:
        break;  

deo.release(); 
Output.release(); 

cv.waitKey(); 
