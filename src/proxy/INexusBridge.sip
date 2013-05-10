%Import QtCore/QtCoremod.sip
%Import QtGui/QtGuimod.sip


namespace MOBase
{

class INexusBridge : public QObject
{

%TypeHeaderCode
#include <inexusbridge.h>
%End

public:

  virtual void requestDescription(int modID, QVariant userData) = 0;
  virtual void requestFiles(int modID, QVariant userData) = 0;
  virtual void requestFileInfo(int modID, int fileID, QVariant userData) = 0;
  virtual void requestDownloadURL(int modID, int fileID, QVariant userData) = 0;
  virtual void requestToggleEndorsement(int modID, bool endorse, QVariant userData) = 0;

signals:

	void nxmDescriptionAvailable(int modID, QVariant userData, QVariant resultData);
	void nxmFilesAvailable(int modID, QVariant userData, QVariant resultData);
	void nxmFileInfoAvailable(int modID, int fileID, QVariant userData, QVariant resultData);
	void nxmDownloadURLsAvailable(int modID, int fileID, QVariant userData, QVariant resultData);
	void nxmEndorsementToggled(int modID, QVariant userData, QVariant resultData);
	void nxmRequestFailed(int modID, QVariant userData, const QString &errorMessage);

};

};
