#deklarace pouzitych "globalnich" promennych
minutesAM <- 6 * 60 #pocet minut po ktere prichazi lide do stanic A, B (dopoledne)
avgIntervalA <- 1/40 #stredni doba mezi prichody lidi na stanici A
avgIntervalB <- 1/30 #stredni doba mezi prichody lidi na stanici B
lambdaA <- 40 #hodnota lambda pro exp rozdeleni prichodu lidi do stanice A
lambdaB <- 30  #hodnota lambda pro exp rozdeleni prichodu lidi do stanice B
arrivalsAAM <- NULL #pole prichodu lidi (minuty) do stanice A (dopoledne)
arrivalsBAM <- NULL #pole prichodu lidi (minuty) do stanice B (odpoledne)
nAM <- minutesAM / 2 #pocet jizd vlaku dopoledne 

set.seed(13031987)

# cas prichodu prvniho cloveka (v minutach) do stanice A - generovani 1 hodnoty 
# z exp rozdeleni s vypoctenym parametrem lambdaA
time <- rexp(1,lambdaA)  

#smycka generujici prichody lidi do stanice A (dopoledne - od 6:00)
while(time <= minutesAM)
{
  arrivalsAAM <- c(arrivalsAAM,time)  #ulozit cas prichodu
  time <- time + rexp(1,lambdaA) #vygenerujeme cas prichodu dalsiho cloveka
}

#smycka generujici prichody lidi do stanice B (dopoledne - od 6:05)
time <- rexp(1,lambdaB) 
while(time <= minutesAM)
{
  arrivalsBAM <- c(arrivalsBAM,time)  #ulozit cas prichodu
  time <- time + rexp(1,lambdaB) #vygenerujeme cas prichodu dalsiho cloveka
}

peopleCountAAM <- length(arrivalsAAM) #pocet lidi kteri prisli do stanice A dopoledne
peopleCountBAM <- length(arrivalsBAM) #pocet lidi kteri prisli do stanice B dopoledne

#ukol 1.1 - pocet lidi jedoucich prvnim vlakem mezi stanicemi B,C
#secteme pocet lidi, kteri prisli do stanice A behem prvnich dvou minut provozu 
#tedy v intervalu [6:00 - 6:02], pricteme pocet lidi, kteri prisli do stanice B
#behem prvnich dvou minut provozu tedy v intervalu [6:05,6:07]
task11FirstTrainCount <- ( length(arrivalsAAM[arrivalsAAM <= 2]) + length(arrivalsBAM[arrivalsBAM <= 2]) )
print(paste(c("Ukol 1.1 - Prvnim vlakem jelo mezi stanicemi B a C ",task11FirstTrainCount," lidi."),sep = " ",collapse = ""))

#ukol 1.2 - odhadnout stredni hodnotu poctu lidi mezi stanicemi B a C dopoledne
task12Counts <- NULL #pocty lidi v jednotlivych dopolednich vlacich mezi B a C, stejny vypocet jako v 1.1
trainTime <- 2 #cas odjezdu vlaku (minuty)
#vygenerujeme obsah task12Counts
while(trainTime <= minutesAM)
{
  task12Counts <- c ( task12Counts, length(arrivalsAAM[(arrivalsAAM > trainTime - 2) & (arrivalsAAM <= trainTime)]) + length(arrivalsBAM[(arrivalsBAM > trainTime - 2) & (arrivalsBAM <= trainTime)]) )
  trainTime <- trainTime + 2
}

#bodovy odhad - odhad stredni hodnoty = vyberovy prumer
task12Mean <- mean(task12Counts) 
task12Var <- var(task12Counts) 
print(paste("Ukol 1.2 - Bodovy odhad: Stredni hodnota poctu lidi dopoledne mezi stanicemi B a C je ", task12Mean, sep=" ",collapse = ""))

#Ukol 1.2 - 95% intervalovy odhad, mame normalni rozdeleni (z 1.5) -> 1 vyberovy t-test 
print("Ukol 1.2 - 1-vyberovy t-test + intervalovy 95% odhad:")
print(paste("(",(t.test(task12Counts))[[4]][1],",",(t.test(task12Counts))[[4]][2],")",collapse="")) 

#Ukol 1.3 - testovat nulovou hypotezu, ze stredni pocet cestujicich dopoledne mezi B a C
#je 140. Opet s vyuzitim znalosti, ze pocty cesztujicich maji normalni rozdeleni.
print(t.test(task12Counts, mu = 140))
print("=> hypotezu nezamitam")

#ukol 1.4, 1.6 - nakreslit histogram poctu cestujicich dopoledne mezi B a C, interval = 10 lidi
hist(task12Counts, breaks = 10, main = "Úkol 1.4 - histogram",freq=FALSE)
lines(density(task12Counts))

#ukol 1.5 - test normality rozdeleni
print("Ukol 1.5 - test normality rozdeleni:")
print(shapiro.test(task12Counts))
print("=> Hypotezu nezamitam")

#ukol 1.7 - hledani rozdeleni doby cekani na vlak
#zkonstruujeme vektor dob cekani na vlak z A do B, z B do C dopoledne
task17WaitTimes <- c((2 - (arrivalsAAM %% 2)),(2 - (arrivalsBAM %% 2)))
#tvrzeni dolozime histogramem
hist(task17WaitTimes, breaks = 10, freq = FALSE, main = "Úkol 1.7 - Èekání na vlak má rovnomìrné rozdìlení" )
lines(density(task17WaitTimes))

#ukol 1.8 - navrhnout kapacitu vlaku - vime, ze pocty lidi maji normalni rozdeleni
# - vezmeme 0.98 kvantil prislusneho rozdeleni
task1.8Capacity <- qnorm(0.98, mean = task12Mean, sd = sqrt(task12Var) )
print(paste("Ukol 1.8 - Hledana kapacita je ",ceiling(task1.8Capacity), sep = " ", collapse = ""))

#ukol 2.0 - nagenerujeme prichody cestujicich do stanice C odpoledne
#nejdriv spocitame lidi, kteri prisli dopoledne
peopleTotal <- peopleCountAAM + peopleCountBAM
#vygenerujeme casy prichodu lidi - normalni rozdeleni
minutesPM <- 8 * 60 #doba odpoledniho probozu v minutach
arrivalsCPM <- sort(runif(n = peopleTotal, min = 0, max = minutesPM))
task21Deltas <- arrivalsCPM - c(0,arrivalsCPM[1:length(arrivalsCPM) - 1])
#zjistime rozdeleni z histogramu
hist(task21Deltas, breaks = 10, freq = FALSE, main = "Úkol 2.1 - Doba mezi pøíchody má exponenciální rozdìlení")
lines(density(task21Deltas))

#ukol 2.2 - odhad stredni doby poctu cestujicich odpoledne mezi stanicemi C a B
task22Counts <- NULL #pocty lidi v jednotlivych odpolednich vlacich mezi stanicemi C a B
trainTime <- 2 #cas odjezdu vlaku (minuty)
#vygenerujeme obsah task12Counts
while(trainTime <= minutesPM)
{
  task22Counts <- c ( task22Counts, length(arrivalsCPM[(arrivalsCPM > trainTime - 2) & (arrivalsCPM <= trainTime)]))
  trainTime <- trainTime + 2
}

#bodovy odhad
print(paste("Ukol 2.2 - bodovy odhad: stredni hodnota poctu lidi odpoledne mezi stanicemi C a B je: ",mean(task22Counts),collapse=""))

#intervalovy odhad


#ukol 2.3 - rozdeleni celkove doby cekani na vlak jednoho cloveka za jeden den
task23WaitTimes <- 2 - (arrivalsCPM %% 2) # vektor dob cekani lidi na odpoledni vlak
task23TotalWaitTimes <- task17WaitTimes + task23WaitTimes
#zjistime rozdeleni z histogramu
hist(task23TotalWaitTimes,breaks = 100, freq = FALSE, main = "Úkol 2.3 - celková doba èekání má normální rozdìlení")
lines(density(task23TotalWaitTimes))