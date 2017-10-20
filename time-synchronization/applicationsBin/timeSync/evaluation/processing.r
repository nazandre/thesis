getHistogram <- function (v) {
	a <- table(v)
	f <- as.data.frame(a)
	x <- c(sort(unique(v)))
	y <-f$Freq/length(v)*100

	#print(f)

	dat <- data.frame(y,x)
	expanded <- with(dat, rep(x,y))
	cat("========\n")
	cat("mean: ", mean(expanded), "\n")
	cat("sd: ", sd(expanded), "\n")

	dataToExport <- data.frame(f$v,y)
	da2 <-as.matrix(dataToExport)
	da3 <- matrix(da2, ncol=ncol(da2),dimnames = NULL)
	names(da3) <- NULL
	
	return (da3)
}

file <- commandArgs(TRUE)[1]
data <- read.table(file)

processing <- data[,3]/1000

#oneway <- oneway / 0.991386

hProcessing <- getHistogram(processing)

write.table(hProcessing,  quote = FALSE, file = "processing/distribution.dat", sep=" ",col.names=FALSE,row.names = FALSE)