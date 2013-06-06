%Import QtCore/QtCoremod.sip
%Import QtGui/QtGuimod.sip


namespace MOBase
{

class ModRepositoryFileInfo
{

%TypeHeaderCode
#include <imodrepositorybridge.h>
%End

public:

  ModRepositoryFileInfo();
  ModRepositoryFileInfo(const QString &data);
  ~ModRepositoryFileInfo();

  QString toString() const;

};


class IModRepositoryBridge : QObject
{

%TypeHeaderCode
#include <imodrepositorybridge.h>
%End

public:

  virtual void requestDescription(int modID, QVariant userData) = 0;
  virtual void requestFiles(int modID, QVariant userData) = 0;
  virtual void requestFileInfo(int modID, int fileID, QVariant userData) = 0;
  virtual void requestDownloadURL(int modID, int fileID, QVariant userData) = 0;
  virtual void requestToggleEndorsement(int modID, bool endorse, QVariant userData) = 0;

signals:

  void descriptionAvailable(int modID, QVariant userData, QVariant resultData);
  void filesAvailable(int modID, QVariant userData, const QList<MOBase::ModRepositoryFileInfo> &resultData);
  void fileInfoAvailable(int modID, int fileID, QVariant userData, QVariant resultData);
  void downloadURLsAvailable(int modID, int fileID, QVariant userData, QVariant resultData);
  void endorsementToggled(int modID, QVariant userData, QVariant resultData);
  void requestFailed(int modID, QVariant userData, const QString &errorMessage);

};

};
