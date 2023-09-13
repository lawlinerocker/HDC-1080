import { StatusBar } from 'expo-status-bar';
import { StyleSheet, Text, View, Image,ImageBackground } from 'react-native';
import { initializeApp } from 'firebase/app';
import { getDatabase, ref, onValue, set } from 'firebase/database';
import { useState,useEffect } from 'react'
import { useFonts } from 'expo-font';
import { useCallback } from 'react';
export default function App() {

  const firebaseConfig = {
    apiKey: "AIzaSyBXaWBweUFj274vs3-Kxj1bVl4JD9-qJfc",
    authDomain: "projectembeded3.firebaseapp.com",
    databaseURL: "https://projectembeded3-default-rtdb.asia-southeast1.firebasedatabase.app",
    projectId: "projectembeded3",
    storageBucket: "projectembeded3.appspot.com",
    messagingSenderId: "430734841812",
    appId: "1:430734841812:web:14bb850b824726be652841",
    measurementId: "G-55G0V6HHGJ"
  };

  const app = initializeApp(firebaseConfig);

  const db = getDatabase();
  // const starCountRef = ref(db, 'posts/' + postId + '/starCount');
  // onValue(starCountRef, (snapshot) => {
  //   const data = snapshot.val();
  //   updateStarCount(postElement, data);
  // });
  const [temp,setTemp] = useState(0)
  const [humid,setHumid] = useState(0)
  const getData = ref(db,"/now");

  useEffect(()=>{
    onValue(getData, (snapshot)=>{
      //console.log(snapshot.val());
      const data = snapshot.val();
      setTemp(Math.floor(data.temperature*100)/100);
      setHumid(Math.floor(data.humidity*100)/100);
    })
  },[temp,humid])
  const pic_temp = {uri:"https://i.ibb.co/rwN9Yw9/temperature.png"}
  const pic_humid = {uri:"https://i.ibb.co/KjQct1q/humidity.png"}


  const [fontsLoaded] = useFonts({
    'Sound-Rounded': require('./assets/fonts/FC-Sound-Rounded.ttf'),
  });

  const onLayoutRootView = useCallback(async () => {
    if (fontsLoaded) {
      await SplashScreen.hideAsync();
    }
  }, [fontsLoaded]);

  if (!fontsLoaded) {
    return null;
  }
  const gifbackground = {uri:"https://media.giphy.com/media/U3qYN8S0j3bpK/giphy.gif"}
  return (
    <ImageBackground source={gifbackground} style={{justifyContent:'center',alignItems:'center',flex:1,backgroundColor:'black'}} onLayout={onLayoutRootView}>
      <View style={{width:400,height:350,borderColor:'red',borderWidth:0,borderRadius:30,backgroundColor:'white',justifyContent:'center',alignItems:'center'}}>
        <Text style={{fontSize:35,marginLeft:10,fontFamily:'Sound-Rounded'}}>Temperature° </Text>
          <View style={{flex:1,borderWidth:0,justifyContent:'center',alignItems:'center',flexDirection:'row'}}>
          
            <Image source={pic_temp} style={{width:100,height:100}}/>
            <Text style={{fontSize:35,marginLeft:10,fontFamily:'Sound-Rounded'}}>{temp} °C</Text>
          </View>
          <Text style={{fontSize:35,marginLeft:10,fontFamily:'Sound-Rounded'}}>Relative Humidity %</Text>
          <View style={{flex:1,borderWidth:0,justifyContent:'center',alignItems:'center',flexDirection:'row'}}>
           
            <Image source={pic_humid} style={{width:100,height:100}}/>
            <Text style={{marginRight:10,fontSize:35,fontFamily:'Sound-Rounded'}}>{humid} %</Text>
          </View>
        </View>
      <StatusBar style="auto" />
    </ImageBackground>

  );
}


