package network

import (
	"eosex/network/config"
	"github.com/emicklei/go-restful"
	"log"
	"net/http"
	"sync"
)

const detailsLimit uint8 = 10

type ErrorDetail struct {
	Message    string `json:"message"`
	File       string `json:"file"`
	LineNumber uint64 `json:"line_number"`
	Method     string `json:"method"`
}

type ErrorInfo struct {
	Code    int64         `json:"code"`
	Name    string        `json:"name"`
	What    string        `json:"what"`
	Details []ErrorDetail `json:"details"`
}

func NewErrorInfo(code int64, name, what string) *ErrorInfo {
	return &ErrorInfo{
		Code:    code,
		Name:    name,
		What:    what,
		Details: make([]ErrorDetail, 0),
	}
}

type ErrorResults struct {
	Code    uint16    `json:"code"`
	Message string    `json:"message"`
	Error   ErrorInfo `json:"error"`
}

func NewErrorResults(code uint16, msg string, err *ErrorInfo) *ErrorResults {
	return &ErrorResults{
		Code:    code,
		Message: msg,
		Error:   *err,
	}
}

func NewErrorResultsByError(err error) *ErrorResults {
	errInfo := NewErrorInfo(3999999, "error", err.Error())
	return &ErrorResults{
		Code:    500,
		Message: "Internal Service Error",
		Error:   *errInfo,
	}
}

type NoErrorResults struct {
}

var httpOnce sync.Once
var httpMgr *HttpMgr

type HttpMgr struct {
}

func GetHttpMgr() *HttpMgr {
	httpOnce.Do(func() {
		httpMgr = &HttpMgr{}
	})
	return httpMgr
}

func (m *HttpMgr) Startup() {
	wsContainer := restful.NewContainer()
	restMgr := NewRestMgr()
	restMgr.Register(wsContainer)
	log.Printf("start restful listening on localhost%s", config.HttpListenEndpoint)
	httpServer := &http.Server{Addr: config.HttpListenEndpoint, Handler: wsContainer}
	go httpServer.ListenAndServe()
}
