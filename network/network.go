package main

import (
	"eosex/network/network"
	"fmt"
	"os"
	"os/signal"
	"runtime"
	"syscall"
)

func main() {
	runtime.GOMAXPROCS(runtime.NumCPU())

	httpMgr := network.GetHttpMgr()
	httpMgr.Startup()

	network.GetLocalDataInstance()

	connMgr := network.GetConnsMgr()
	connMgr.Startup()

	// catch system signal
	chSig := make(chan os.Signal)
	signal.Notify(chSig, syscall.SIGINT, syscall.SIGTERM)
	fmt.Println("Signal: ", <-chSig)

	connMgr.Stop()

}
