# AS7331
Sample sketch for the [AS7331](https://www.mouser.com/catalog/specsheets/amsOsram_AS7331_DS001047_1-00.pdf) UV A/B/C light sensor by AMS

Easy to use UV A/B/C light sensor from AMS. The well-commented sketch shows how to configure the sensor, make use of the power saving features, data ready interrupt and properly scale the light and chip temperature data while varying the gain and integration time. Basic error handling is included. Should be a good place to start for any custom applications.

![AS7331](https://user-images.githubusercontent.com/6698410/215298161-abc613b3-15e3-4a7d-9e9e-55e6fc9d6ac8.jpg)

Both CMD (one shot) and CONT (continuous) modes work with the latter sample rate defined by the time variable. *gain* and (integration) *time* determine the sensitivity of the sensor to light. For example, use low gain and short integration time for direct sunlight. I used a gain of 1024 and integration time of 512 s to collect the below data from indoors through a window on a cloudy, drizzly day. The sensor is quite sensitive!

![output](https://user-images.githubusercontent.com/6698410/215355679-dce4a8ac-84da-4aaa-996b-fb7d1a03ee3d.jpg)

I haven't made SYNS or SYND modes work yet.

Sketch makes use of the STM32L432 (Ladybug) development board but should work with just about any Arduino compatible dev board with minor modifications.

Breakout board design can be found in the [OSH Park](https://oshpark.com/shared_projects/UWxzGGvE) shared space.

2023 Copyright Tlera Corporation
