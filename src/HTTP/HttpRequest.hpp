#pragma once

#include "../ConfigFile/Server.hpp"
#include "../EventPool/EventPool.hpp"
#include "AHttp.hpp"

class Server;
class Location;
struct eventStructTmp;

class HttpRequest : public AHttp {
private:
  RequestType _type;
  bool _badRequest;
  str _receivedMethod;
  str _resource;
  bool _resourceExists;
  bool _validMethod;
  bool _isCgi;
  str _localPathResource;
  Location *_location;
  str _locationUri;
  str _queryString;
  str _pathInfo;
  str _redirect;
  bool _autoIndex;
  bool _canAccess;
  str _sessionUser;
  bool _payLoad;

  void parse();
  void checkHeaderMRP(const str &line);
  const str saveHeader(const str &request);
  bool checkResource(Server const &server);
  void parseResource();
  str addPathInfo(str afterSrc);
  bool checkAllowMethod();

  void addUser(strMap &header);

public:
  HttpRequest(eventStructTmp *eventstrct);
  ~HttpRequest();
  Location *findLocation(Server *Server);
  void autoIndex(Location *loc);
  bool checkFileExists(str fullResource);
  bool locationHasRedirection(const Location *loc);

  RequestType getType() const;
  bool getBadRequest() const;
  bool getResourceExists() const;
  bool getValidMethod() const;
  bool getIsCgi() const;
  bool getIsValidCgi() const;
  Location *getLocation() const;
  str getVarCgi() const;
  bool getHeaderTooLarge() const;
  str getReceivedMethod() const;
  str getResource() const;
  str getLocalPathResource() const;
  str getLocationUri() const;
  str getQueryString() const;
  str getPathInfo() const;
  str getRedirect() const;
  bool getAutoIndex() const;
  bool getCanAccess() const;
  str getSessionUser() const;
  bool getPayLoad() const;

  void setType(RequestType type);
  void setBadRequest(bool badRequest);
  void setResorceExist(bool resorceExist);
  void setValidMethod(bool validMethod);
  void setIsCgi(bool isCgi);
  void setIsValidCgi(bool isValidCgi);
  void setLocation(Location *location);
  void setVarCgi(str varCgi);

  class badHeaderException : public std::exception {
  private:
    str _msg;

  public:
    badHeaderException(const str &msg);
    virtual const char *what() const throw();
    virtual ~badHeaderException() throw(){};
  };
};
