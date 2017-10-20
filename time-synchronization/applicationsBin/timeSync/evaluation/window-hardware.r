# execute script: Rscript window.r

#exp <- c("exp11", "exp12", "exp13", "exp2","exp7","exp8","exp9")
#window <- c(1,2,3,5,10,20,30)

dir <- "hardware/window/"
exp <- c("exp12", "exp13", "exp2","exp7","exp8","exp9")
window <- c(2,3,5,10,20,30)

windowSummary <- matrix(nrow=length(exp),ncol=6)

for (i in 1:length(exp)){
    file <- paste(dir, exp[i], "/global-filtered.dat", sep="")
    cat("file: ", file, "\n")
    clockData <- read.table(file)   
    global <- clockData[,2]
    estimated <- clockData[,4]
    error <- (estimated - global)

    filter <- 1000*60*60
    test <- estimated[ estimated < filter ]
    error <- error[5:length(test)]

    # Proba
    print(summary(abs(error)))
    print(sd(error))
   
    absError <- error
    
    #absError <- abs(error)
    
    windowSummary[i,1] = as.numeric(window[i])
    windowSummary[i,2] = mean(absError)
    windowSummary[i,3] = min(absError)
    windowSummary[i,4] = max(absError)
    windowSummary[i,5] = sd(absError)
    windowSummary[i,6] = quantile(sort(absError),c(.99))
   
    a <- table(error)
    f <- as.data.frame(a)
    x <- c(sort(unique(error)))
    y <-f$Freq/length(error)*100
    
    total <- sum(y)
    cat("total: ", total, "\n")
    
    dataToExport <- data.frame(x,y)
    da2 <-as.matrix(dataToExport)
    da3 <- matrix(da2, ncol=ncol(da2),dimnames = NULL)
    names(da3) <- NULL
    write.table(da3,  quote = FALSE, file = paste(dir,window[i],"w.dat",sep=""), sep=" ",col.names=FALSE,row.names = FALSE)
}
   windowSummary <- windowSummary[sort.list(windowSummary[,1]), ]
   print("Summary:")
   print(windowSummary)
   write.table(windowSummary, quote = FALSE, file = paste(dir,"summary.dat",sep=""), sep=" ", col.names=FALSE, row.names = FALSE)
