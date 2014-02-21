#https://github.com/VikParuchuri/simpsons-scripts/blob/master/analyze_sentiment.R
if (!require("RColorBrewer")) {
install.packages("RColorBrewer")
library(RColorBrewer)
}
args <- commandArgs()
print(args[7])
ll <- read.csv(args[5],sep=",",check.names=FALSE)
row.names(ll) <- ll[,1]
nc <-ncol(ll)
ll <- ll[,2:nc]
ll_matrix <- data.matrix(ll)
col = brewer.pal(11,"RdYlGn")
col2 = brewer.pal(ncol(ll_matrix),"RdYlGn")
myBreaks=c(0, 33, 50, 75, 85, 97, 103, 110, 130, 150, 200, 100000)
#now to save in pdf
pdf(file=args[6])
#ll_heatmap <- heatmap(ll_matrix, Rowv=NA, Colv=NA, col=col, scale="column", margins=c(5,10))
ll_heatmap <- heatmap(ll_matrix, Rowv=NA, Colv=NA, col=col, scale="none", ylab=args[9], xlab=args[8], margins=c(5,5), ColSideColors=col2, main=args[7], breaks=myBreaks)
dev.off()
