---
title: "Maven NYC Taxi Challenge"
author: "Abe"
date: "10/17/2021"
output: pdf_document
---
Calling packages
Using the tidyverse because it has the libraries I need to clean, mutate and visualize data.
library(data.table) - helps load data faster then read csv
```{r}
library(tidyverse)
library(data.table)
library(lubridate)
```

Loading all of the excel files into Rstudio
```{r}
taxi_data_2017 <- fread("C:/Users/abedi/OneDrive/Desktop/NYC_Taxi_Trip R_project/taxi_trips/2017_taxi_trips.csv")
taxi_data_2018 <- fread("C:/Users/abedi/OneDrive/Desktop/NYC_Taxi_Trip R_project/taxi_trips/2018_taxi_trips.csv")
```


```{r}
taxi_data_2019 <- fread("C:/Users/abedi/OneDrive/Desktop/NYC_Taxi_Trip R_project/taxi_trips/2019_taxi_trips.csv")
taxi_data_2020 <- fread("C:/Users/abedi/OneDrive/Desktop/NYC_Taxi_Trip R_project/taxi_trips/2020_taxi_trips.csv")
```

Noticed taxi_data_2019 & 2020 have 19 columns checked the first 6 rows to see what it is. 
```{r}
head(taxi_data_2019)
```
```{r}
head(taxi_data_2020)
```
Congestion_surcharge is the extra column which will prevent the union_all from working because all of the columns need to be the same. Going to remove the extra columns for both 2019 & 2020.
```{r}
taxi_data_2019$congestion_surcharge <- NULL
```

```{r}
taxi_data_2020$congestion_surcharge <- NULL
```

**Did not remove any columns as previously stated because removing them caused an issued with the union_all.**

At first I tried to union_all the 4 dataframes which took too long which resulted in the total rows being 8M less. I decided to conduct two separate union_all statements to fix the issue. 
```{r}
taxi_data_all <- union_all(taxi_data_2017, taxi_data_2018)
```

```{r}
taxi_data_all_2 <- union_all(taxi_data_2019, taxi_data_2020)
```

```{r}
taxi_data_all_final <- union_all(taxi_data_all, taxi_data_all_2)
```

Removed previous dataframes to save GB in Global Environment
```{r}
remove(taxi_data_2017, taxi_data_2018, taxi_data_2019, taxi_data_2020, taxi_data_all, taxi_data_all_2)
```

Garbage collection function to print memory usage statistics as previously I was receiving this error 
**Error: cannot allocate vector of size 216.1 Mb**
```{r}
gc()
```

Checked memory limit and noticed it was 12049
```{r}
memory.limit()
```

Increased limit as noted below and ran the final union_all again and summed up the dataframes to compare the final union_all to confirm 28M rows were unioned successfully.
```{r}
memory.limit(size = 50000)
```
View data types
```{r}
glimpse(taxi_data_all_final)
```

Cleaning requirements:

1. Remove trips that were NOT sent via "store and forward".
```{r}
taxi_data_clean <- taxi_data_all_final %>% 
  filter(store_and_fwd_flag == "N")
```

```{r}
remove(taxi_data_all_final)
```


2. Only keep street-hailed trips paid by card or cash with a standard rate.
```{r}
taxi_data_clean <- taxi_data_clean  %>% 
  filter(RatecodeID == 1 & (payment_type < 3))
#RateCodeID & payment_type are both int
```

```{r}
head(taxi_data_clean)
```

3. Remove any trips before 2017 or after 2020, along with any trips or pickups or drop-offs into unknown zones.
```{r}
taxi_data_clean <- taxi_data_clean  %>% 
  filter(between (lpep_pickup_datetime, as.Date("2017-01-01"), as.Date ("2021-01-01")))
```

4. Any trips with no recorded passengers had 1 passenger.
```{r}
taxi_data_clean <- taxi_data_clean  %>% 
mutate(passenger_count = if_else(passenger_count == 0, 1, 1)) 
```

Verify it worked ^
```{r}
min(taxi_data_clean$passenger_count)
```
5. If pickup date/time is AFTER drop-off date/time, swap them.
```{r}
taxi_data_clean <- taxi_data_clean  %>% 
mutate(lpep_pickup_datetime = if_else(lpep_dropoff_datetime < lpep_pickup_datetime, lpep_dropoff_datetime, lpep_pickup_datetime))
```

Cleaning step worked ^
```{r}
taxi_data_clean %>% 
filter(lpep_dropoff_datetime < lpep_pickup_datetime)
```


6. Remove any trips lasting longer than a day, and any trips which show a distance and fare amount of zero.
```{r}
taxi_data_clean <- taxi_data_clean %>% 
  mutate(trip_hours = difftime(taxi_data_clean$lpep_dropoff_datetime, taxi_data_clean$lpep_pickup_datetime, units = "hours"))
```


Verifying the time difference
```{r}
max(taxi_data_clean$trip_hours)
```

```{r}
taxi_data_clean <- taxi_data_clean  %>% 
  filter(trip_hours <= 24) %>% 
  filter(trip_distance > 0 & (fare_amount > 0))
```

Verifying that anything greater then a day and removed anything that was 0. 
```{r}
max(taxi_data_clean$trip_hours)
min(taxi_data_clean$trip_distance)
min(taxi_data_clean$fare_amount)
```


7. If any fare, taxed and surcharge are negative make positive.
```{r}
min(taxi_data_clean$fare_amount)
min(taxi_data_clean$mta_tax)
min(taxi_data_clean$congestion_surcharge)
summary(taxi_data_clean$congestion_surcharge)
```

Saving the final clean copy of data to use in separate r markdown. Pedro recommends always have 2 markdowns one for cleaning and one for visualizing. 
```{r}
write.csv(taxi_data_clean, "C:/Users/abedi/OneDrive/Documents/R Projects/Maven Analytics NYC Taxi Challenge/clean_data.csv")
```

