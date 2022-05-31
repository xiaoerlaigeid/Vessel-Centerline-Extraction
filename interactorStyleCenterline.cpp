#include "interactorStyleCenterline.h"

void MouseInteractorStyleCenterline::OnKeyPress()
{
	vtkRenderWindowInteractor *rwi = this->Interactor;
	std::string key = rwi->GetKeySym();

	if (key == "n" || key=="N")
	{
		if ((m_currentSeedPosition[0] != 0 && m_currentSeedPosition[1] != 0 && m_currentSeedPosition[2] != 0) || m_numOfSeeds == 0)
		{
			if (m_currentSeedType == 0)
				m_numOfSourceSeed++;
			else
				m_numOfTargetSeed++;
			m_numOfSeeds = m_numOfSourceSeed + m_numOfTargetSeed;
			cout << "press n m_numOfSeeds is " << m_numOfSeeds << endl;
			// Create a sphere
			vtkSmartPointer<vtkSphereSource> sphereSource = vtkSmartPointer<vtkSphereSource>::New();
			double sphereCenter[3];
			sphereSource->SetCenter(0, 0, 0);
			sphereSource->SetRadius(0.5);
			seedList.insert(m_numOfSeeds, sphereSource);

			m_currentSeedPosition[0] = sphereSource->GetCenter()[0];
			m_currentSeedPosition[1] = sphereSource->GetCenter()[1];
			m_currentSeedPosition[2] = sphereSource->GetCenter()[2];

			vtkSmartPointer<vtkPolyDataMapper> sphereMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
			sphereMapper->SetInputConnection(sphereSource->GetOutputPort());

			vtkSmartPointer<vtkActor> sphereActor = vtkSmartPointer<vtkActor>::New();
			sphereActor->SetMapper(sphereMapper);
			if (m_currentSeedType == 0) sphereActor->GetProperty()->SetColor(1, 0, 0);
			else						sphereActor->GetProperty()->SetColor(0, 1, 0);

			seedActorList.insert(m_numOfSeeds, sphereActor);
			seedTypeList.insert(m_numOfSeeds, m_currentSeedType);
			this->GetCurrentRenderer()->AddActor(sphereActor);
		}
		else
			cout << "Invalid seed position, no new seed is inserted" << endl;
	}
	else if (key == "Tab")
	{
		// switch seed type
		m_currentSeedType = !m_currentSeedType;
		if (m_currentSeedType == 0)
		{
			cout << "Current seed type is source" << endl;
			if (m_numOfSeeds > 0)
			{
				m_numOfSourceSeed++;
				if (m_numOfTargetSeed > 0)
					m_numOfTargetSeed--;
				seedTypeList[m_numOfSeeds] = m_currentSeedType;
				this->GetCurrentRenderer()->GetActors()->GetLastActor()->GetProperty()->SetColor(1, 0, 0);
			}
		}
		else
		{
			cout << "Current seed type is target" << endl;
			if (m_numOfSeeds > 0)
			{
				if (m_numOfSourceSeed>0)
					m_numOfSourceSeed --;
				m_numOfTargetSeed++;
				seedTypeList[m_numOfSeeds] = m_currentSeedType;
				this->GetCurrentRenderer()->GetActors()->GetLastActor()->GetProperty()->SetColor(0, 1, 0);
			}	
		}
		cout << "Number of source seeds = " << m_numOfSourceSeed << ", number of target seeds = " << m_numOfTargetSeed << endl;
	}
	else if (key == "space")
	{
		//先使用空格进行选择, 然后再用tab键调整位置，最后用n
		this->Interactor->GetPicker()->Pick(this->Interactor->GetEventPosition()[0],
			this->Interactor->GetEventPosition()[1],
			0,  // always zero.
			this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer());
		double picked[3];
		this->Interactor->GetPicker()->GetPickPosition(picked);
		m_currentSeedPosition[0] = picked[0];
		m_currentSeedPosition[1] = picked[1];
		m_currentSeedPosition[2] = picked[2];
		if (m_numOfSeeds != 0)
		{
			cout << "insert m_numOfSeeds is " << m_numOfSeeds << endl;
			seedList[m_numOfSeeds]->SetCenter(picked);
		}
		cout <<"picked is "<< picked[0] << "," << picked[1] << "," << picked[2] << endl;
	}
	else if (key == "Return")
	{
		cout << "press Return" << std::endl;
		if (m_numOfSourceSeed == 0 && m_numOfTargetSeed==0)
			cout << "Source/target seed not found" << endl;
		else if (m_numOfSourceSeed == 0 && m_numOfTargetSeed>0)
			cout << "Source seed not found" << endl;
		else if (m_numOfSourceSeed > 0 && m_numOfTargetSeed==0)
			cout << "Target seed not found" << endl;
		else if (m_currentSeedPosition[0] == 0 && m_currentSeedPosition[1] == 0 && m_currentSeedPosition[2] == 0)
			cout << "Invalid seed position, cannot calculate centerline" << endl;
		else
		{
			cout << "Centerline calculation in progress" << endl;
			// Create the kd tree
			vtkSmartPointer<vtkKdTreePointLocator> kDTree = vtkSmartPointer<vtkKdTreePointLocator>::New();
			kDTree->SetDataSet(m_surface);
			kDTree->BuildLocator();

			vtkSmartPointer<vtkIdList> sourceIds = vtkSmartPointer<vtkIdList>::New();
			sourceIds->SetNumberOfIds(m_numOfSourceSeed);
			vtkSmartPointer<vtkIdList> targetIds = vtkSmartPointer<vtkIdList>::New();
			targetIds->SetNumberOfIds(m_numOfTargetSeed);

			int _sourceSeedCount = 0;
			int _targetSeedCount = 0;
			cout << "m_numOfSeeds is " << m_numOfSeeds << endl;
			for (int i = 1; i <= m_numOfSeeds; i++)
			{	
				cout << "i is " << i << endl;
				// Find the closest point ids to the seeds
				cout << seedList.size() << endl;
				double pos[3];
				seedList[i]->GetCenter(pos);
				cout << pos[0] << "," << pos[1] << "," << pos[2];
				cout << seedList[i]->GetCenter()[0] << "," << seedList[i]->GetCenter()[1] << "," << seedList[i]->GetCenter()[2] << endl;;
				vtkIdType iD = kDTree->FindClosestPoint(seedList[i]->GetCenter());
				std::cout << "The closest point is point " << iD << std::endl;
				if (seedTypeList[i] == 0)
				{
					sourceIds->SetId(_sourceSeedCount, iD);
					_sourceSeedCount++;
				}
				else
				{
					targetIds->SetId(_targetSeedCount, iD);
					_targetSeedCount++;
				}
				seedActorList[i ]->SetVisibility(0);
			}

			cout << "start Centerline" <<  endl;
			Centerline* centerline = new Centerline;
			centerline->SetCappedSurface(m_surface);
			centerline->SetSourceIds(sourceIds);
			centerline->SetTargetIds(targetIds);
			centerline->SetAppendEndPoints(m_appendFlag);
			centerline->Update();
			m_centerline->DeepCopy(centerline->GetCenterline());
			cout << "m_centerline->GetNumberOfPoints()"<<m_centerline->GetNumberOfPoints() << endl;

			// set surface opacity
			vtkActor* actor = vtkActor::SafeDownCast(this->GetCurrentRenderer()->GetActors()->GetItemAsObject(0));
			actor->GetProperty()->SetOpacity(0.5);
		}
	}
	this->GetInteractor()->Render();
}

void MouseInteractorStyleCenterline::SetCenterline(vtkPolyData* centerline)
{
	m_centerline = centerline;
}

void MouseInteractorStyleCenterline::SetSurface(vtkPolyData* surface)
{
	m_surface = surface;
}

void MouseInteractorStyleCenterline::SetAppendEndPoints(bool appendFlag)
{
	m_appendFlag = appendFlag;
}

vtkStandardNewMacro(MouseInteractorStyleCenterline);
