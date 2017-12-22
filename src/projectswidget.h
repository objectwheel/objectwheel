#ifndef PROJECTSWIDGET_H
#define PROJECTSWIDGET_H

#include <QQuickWidget>
#include <QAbstractListModel>
#include <QHash>
#include <QByteArray>
#include <QList>
#include <QJsonObject>

class ProjectsWidget : public QQuickWidget
{
		Q_OBJECT
	public:
		explicit ProjectsWidget(QWidget *parent = 0);
        static ProjectsWidget* instance();
        static void refreshProjectList(const QString& activeProject = QString());

	public slots:
		void handleNewButtonClicked();
		void handleLoadButtonClicked();
		void handleInfoButtonClicks(const QVariant& projectname);
		void handleBtnCancelClicked();
		void handleBtnDeleteClicked();
		void handleBtnOkClicked();
        void handleBtnImportClicked();
        void handleBtnExportClicked();

    private slots:
        void startProject();

    signals:
        void done();
        void busy(const QString& text);
};

class ProjectListModel : public QAbstractListModel
{
		Q_OBJECT
		Q_DISABLE_COPY(ProjectListModel)

	public:
		enum Roles
		{
			ProjectNameRole = Qt::UserRole + 1,
			LastEditedRole = Qt::UserRole + 2,
			ActiveRole = Qt::UserRole + 3
		};

		struct ProjectProperty
		{
			QString projectName;
			QString lastEdited;
			bool active;
		};

		explicit ProjectListModel(QObject* parent = 0) : QAbstractListModel(parent)
		{
			m_roleNames[ProjectNameRole] = "projectName";
			m_roleNames[LastEditedRole] = "lastEdited";
			m_roleNames[ActiveRole] = "active";
		}

		~ProjectListModel() { }

		QHash<int, QByteArray> roleNames() const override
		{
			return m_roleNames;
		}

		virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override
		{
			Q_UNUSED(parent);
			return _data.size();
		}

		virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override
		{
			QVariant rv;

			if (index.row() >= _data.size())
				return rv;

			switch (role)
			{
				case ProjectNameRole:
					rv = _data.at(index.row()).projectName;
					break;
				case LastEditedRole:
					rv = _data.at(index.row()).lastEdited;
					break;
				case ActiveRole:
					rv = _data.at(index.row()).active;
					break;
				default:
					break;
			}
			return rv;
		}

		virtual QModelIndex index(int row, int, const QModelIndex& parent = QModelIndex()) const override
		{
			Q_UNUSED(parent)
			return createIndex(row, 0);
		}

		virtual bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override
		{
			if (count < 1 || row < 0 || row > rowCount(parent))
				return false;

			beginInsertRows(QModelIndex(), row, row + count - 1);

			for (int r = 0; r < count; ++r)
				_data.insert(row, ProjectProperty());

			endInsertRows();

			return true;
		}

		virtual bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override
		{
			if (count <= 0 || row < 0 || (row + count) > rowCount(parent))
				return false;

			beginRemoveRows(QModelIndex(), row, row + count - 1);

			for (int r = 0; r < count; ++r)
				_data.removeAt(row);

			endRemoveRows();

			return true;
		}

		virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override
		{
			if (index.row() >= 0 && index.row() < _data.size()
				&& m_roleNames.keys().contains(role)) {
				switch (role)
				{
					case ProjectNameRole:
						_data[index.row()].projectName = value.toString();
						break;
					case LastEditedRole:
						_data[index.row()].lastEdited = value.toString();
						break;
					case ActiveRole:
						_data[index.row()].active = value.toBool();
						break;
					default:
						break;
				}
				emit dataChanged(index, index, QVector<int>() << role);
				return true;
			}
			return false;
		}

		virtual void clear()
		{
			emit beginResetModel();
			_data.clear();
			emit endResetModel();
		}

		Qt::ItemFlags flags(const QModelIndex &index) const override
		{
			if (!index.isValid())
				return QAbstractListModel::flags(index);

			return QAbstractListModel::flags(index) | Qt::ItemIsEditable;
		}

		Q_INVOKABLE QJsonObject get(int row) const // For QML side usage
		{
			QJsonObject jobj;
			if (row >= 0 && row < _data.size()) {
				jobj[m_roleNames[ProjectNameRole]] = _data.at(row).projectName;
				jobj[m_roleNames[LastEditedRole]] = _data.at(row).lastEdited;
				jobj[m_roleNames[ActiveRole]] = _data.at(row).active;
				return jobj;
			} else {
				return jobj;
			}
		}

		Q_INVOKABLE QVariant get(int row, const QString& property) const
		{
			int role = roleFromString(property);
			if (role >= 0 && row >= 0)
				return data(index(row, 0), role);
			else
				return QVariant();
		}

		Q_INVOKABLE bool set(int row, const QString& property, const QVariant& data)
		{
			int role = roleFromString(property);
			if (role >= 0 && row >= 0) return setData(index(row, 0), data, role);
			else return false;
		}

		int roleFromString(const QString& roleName) const
		{
			return m_roleNames.key(roleName.toLatin1(), -1);
		}

	private:
		QHash<int, QByteArray> m_roleNames;
		QList<ProjectProperty> _data;
};

#endif // PROJECTSWIDGET_H
