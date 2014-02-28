args <- commandArgs()
print(args)
lf <- read.csv(args[5],sep=",",check.names=FALSE)
lb <- read.csv(args[6],sep=",",check.names=FALSE)
colnames(lf) <- "a"
colnames(lb) <- "a"
pdf(file=args[7])
p=seq(0,1,0.01);
maxY = max(lf$a, lb$a)
r=quantile(lf$a,probs=p);
q=quantile(lb$a,probs=p);
plot(p,r,ylim=c(0,maxY),type="l",xlab="percent",ylab="latency(cycles)",col="blue", main=args[8]);
lines(p,q,col="red")
legend('topleft', c("lf","lb") , lty=1, col=c('blue', 'red'), bty='n', cex=.75)
dev.off()
