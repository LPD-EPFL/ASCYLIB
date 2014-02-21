if (!require("RColorBrewer")) {
install.packages("RColorBrewer")
library(RColorBrewer)
}
library(reshape2)
require(gplots)
library(gplots)
require(RColorBrewer)
args <- commandArgs()
print(args[7])
ll <- read.csv(args[5],sep=",",check.names=FALSE)
row.names(ll) <- ll[,1]
nc <-ncol(ll)-1
ll <- ll[,2:nc]
ll_matrix <- data.matrix(ll)
col = brewer.pal(11,"RdYlGn")
col2 = brewer.pal(ncol(ll_matrix),"RdYlGn")
myBreaks=c(0, 33, 50, 75, 85, 97, 103, 110, 130, 150, 200, 1000000)
#now to save in pdf
pdf(file=args[6])
hm <- heatmap.2(ll_matrix, scale="none", Rowv=NA, Colv=NA,
                col =col, ## using your colors,
                cellnote=ll_matrix/100,
                notecol="black",
                breaks = myBreaks, ## using your breaks
                dendrogram = "none",  ## to suppress warnings
                margins=c(5,5), cexRow=1.0, cexCol=1.0, 
                ylab=args[9], xlab=args[8], main=args[7],
                key=FALSE, 
                lhei = c(1.5,10),
                lwid = c(1.5,10),
                trace="none")
dev.off()
